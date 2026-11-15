"""myLinux test harness: boot (or attach to) a qemu DUT and talk NETCONF.

A Dut either *owns* a throwaway qemu boot -- the default, e.g. 'make test'
-- or *attaches* to an already-running one, inside a 'make test-sh' shell
(signalled by $MYTEST_ATTACH).  Owned DUTs boot clean from factory and are
torn down on exit; attached DUTs are left running.
"""
import base64
import json
import os
import shlex
import signal
import socket
import ssl
import subprocess
import sys
import tempfile
import time
import traceback
import urllib.error
import urllib.request
from contextlib import contextmanager
from datetime import datetime

from ncclient import manager

HERE = os.path.dirname(os.path.abspath(__file__))
IMAGES = os.environ.get(
    "MY_IMAGES", os.path.normpath(os.path.join(HERE, "..", "output", "images")))
SSH_PORT = int(os.environ.get("DUT_SSH_PORT", "2222"))
HTTPS_PORT = int(os.environ.get("DUT_HTTPS_PORT", "8443"))
USER = os.environ.get("DUT_USER", "admin")
PASSWORD = os.environ.get("DUT_PASSWORD", "admin")


class Dut:
    """A qemu device under test, reachable on 127.0.0.1:SSH_PORT."""

    def __init__(self, boot_timeout=180):
        self.boot_timeout = boot_timeout
        self.proc = None
        self.workdir = None
        self.console = None       # console log, for boot watch/diagnostics
        self.consock = None       # console UNIX socket, for ./console
        self.qemulog = None
        self.attached = False
        self.nsteps = 0

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, *exc):
        self.stop()
        return False

    def start(self):
        if os.environ.get("MYTEST_ATTACH"):
            self.attached = True
            self.consock = os.environ.get("MYTEST_CONSOLE")
            return
        self._boot()

    def _boot(self):
        qemu = os.path.join(IMAGES, "qemu.sh")
        if not os.path.exists(qemu):
            raise RuntimeError(f"{qemu} not found -- run 'make' first")

        self.workdir = tempfile.mkdtemp(prefix="mylinux-test-")
        self.consock = os.path.join(self.workdir, "console")
        self.console = self.consock + ".log"
        self.qemulog = os.path.join(self.workdir, "qemu.log")
        cfgimg = os.path.join(self.workdir, "config.img")

        # Throwaway config image (clean from factory) and a console on a
        # UNIX socket, tee'd to a log we can watch and ./console can attach.
        env = dict(os.environ, MY_MNT=cfgimg, MY_CONSOLE=self.consock)
        out = open(self.qemulog, "wb")
        self.proc = subprocess.Popen(
            ["./qemu.sh"], cwd=IMAGES, env=env,
            stdin=subprocess.DEVNULL, stdout=out, stderr=out,
            start_new_session=True)
        out.close()

        print(f"DUT console: {self.console}  (tail -f to watch the boot)",
              flush=True)
        try:
            self._wait_ssh()
        except BaseException:
            self.stop()
            raise

    def stop(self):
        if self.attached or not self.proc or self.proc.poll() is not None:
            return
        try:
            grp = os.getpgid(self.proc.pid)
        except ProcessLookupError:
            return
        for sig in (signal.SIGTERM, signal.SIGKILL):
            try:
                os.killpg(grp, sig)
                self.proc.wait(timeout=15)
                return
            except ProcessLookupError:
                return
            except subprocess.TimeoutExpired:
                continue

    def _tail(self, n=50):
        for path in (self.console, self.qemulog):
            try:
                lines = open(path, "rb").read().splitlines()
            except (OSError, TypeError):
                continue
            if lines:
                body = b"\n".join(lines[-n:]).decode(errors="replace")
                return f"--- {path} (last {n} lines) ---\n{body}"
        return "(no console output)"

    def _wait_ssh(self):
        deadline = time.monotonic() + self.boot_timeout
        while time.monotonic() < deadline:
            if self.proc.poll() is not None:
                raise RuntimeError(
                    f"qemu exited early (rc={self.proc.returncode})\n"
                    + self._tail())
            try:
                with socket.create_connection(("127.0.0.1", SSH_PORT), 2) as s:
                    s.settimeout(2)
                    if s.recv(64).startswith(b"SSH-"):
                        return
            except OSError:
                pass
            time.sleep(1)
        raise TimeoutError(
            f"DUT SSH not up on :{SSH_PORT} within {self.boot_timeout}s\n"
            + self._tail())

    def netconf(self):
        """Open a NETCONF session to the DUT (an ncclient manager)."""
        return manager.connect(
            host="127.0.0.1", port=SSH_PORT,
            username=USER, password=PASSWORD,
            device_params={"name": "default"},
            hostkey_verify=False, allow_agent=False, look_for_keys=False,
            timeout=30)

    def http(self, method, path, body=None, auth=(USER, PASSWORD),
             accept="application/yang-data+json", timeout=30):
        """Open an HTTPS request to the DUT's web server, as a web browser
        would; returns the response, a 4xx/5xx one included.

        `body` is sent as JSON.  `auth` is a (user, password) pair for HTTP
        Basic, or None to send no credentials.  The DUT's certificate is
        self-signed, so it is not verified.
        """
        req = urllib.request.Request(
            f"https://127.0.0.1:{HTTPS_PORT}{path}", method=method,
            data=None if body is None else json.dumps(body).encode())
        req.add_header("Accept", accept)
        if body is not None:
            req.add_header("Content-Type", "application/yang-data+json")
        if auth:
            cred = base64.b64encode(":".join(auth).encode()).decode()
            req.add_header("Authorization", f"Basic {cred}")

        try:
            return urllib.request.urlopen(
                req, timeout=timeout, context=ssl._create_unverified_context())
        except urllib.error.HTTPError as err:
            return err

    def reset(self):
        """Reset running-config to the factory baseline before a test.

        mech's factory-reset RPC (RFC 8808) reapplies the factory
        datastore, so every case starts from the same clean state.  We use
        an RPC rather than a datastore copy because clixon accepts only
        running/candidate/startup as a NETCONF copy-config source, and
        because this is the same reset an operator gets.
        """
        from ncclient.xml_ import to_ele

        rpc = '<factory-reset xmlns="urn:ietf:params:xml:ns:yang:ietf-factory-default"/>'
        with self.netconf() as nc:
            nc.dispatch(to_ele(rpc))

    def reboot(self):
        """Restart the DUT and wait for it to come back up.

        Lets a test check what the system does with its *stored* config on
        boot -- the startup->running commit -- not just live edits.
        """
        from ncclient.xml_ import to_ele

        rpc = '<system-restart xmlns="urn:ietf:params:xml:ns:yang:ietf-system"/>'
        try:
            with self.netconf() as nc:
                nc.dispatch(to_ele(rpc))
        except BaseException:
            pass                          # the session drops as it goes down

        # Wait for sshd to disappear (guest going down), then for it to
        # answer again, so we don't match the pre-reboot session.
        deadline = time.monotonic() + 60
        while time.monotonic() < deadline:
            try:
                socket.create_connection(("127.0.0.1", SSH_PORT), 1).close()
                time.sleep(1)
            except OSError:
                break
        self._wait_ssh()

    def copy(self, source, target):
        """NETCONF <copy-config>, e.g. running -> startup."""
        with self.netconf() as nc:
            nc.copy_config(target=target, source=source)

    def ssh(self, cmd, user=USER, password=PASSWORD):
        """Run a shell command on the DUT over SSH; returns (rc, output)."""
        import paramiko

        cli = paramiko.SSHClient()
        cli.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        cli.connect("127.0.0.1", port=SSH_PORT, username=user,
                    password=password, look_for_keys=False, allow_agent=False,
                    timeout=30)
        try:
            _, out, err = cli.exec_command(cmd)
            output = out.read().decode() + err.read().decode()
            rc = out.channel.recv_exit_status()
        finally:
            cli.close()
        return rc, output

    def get_oper(self, subtree, content="nonconfig"):
        """Read state data via <get content="nonconfig">; return reply XML.

        clixon's <get> takes a 'content' attribute: 'nonconfig' answers
        purely from the state callbacks -- the live kernel state, with none
        of the running config merged in -- so a config-true leaf appears
        only once the system has actually applied it, not just because it
        sits in running.  (This clixon has no NMDA <get-data>; the content
        attribute is the equivalent.)  ncclient's get() cannot set that
        attribute, so dispatch the raw <get>.

        `subtree` is an XML subtree-filter selecting the nodes of interest.
        """
        from ncclient.xml_ import to_ele

        rpc = (f'<get xmlns="urn:ietf:params:xml:ns:netconf:base:1.0" content="{content}">'
               f'<filter type="subtree">{subtree}</filter>'
               f'</get>')
        with self.netconf() as nc:
            return nc.dispatch(to_ele(rpc)).xml

    def cli(self, cmd):
        """Run one admin-exec CLI command and return its output.

        The CLI is clixon_cli; fed a command on stdin it echoes the
        prompt and command, runs it, and exits at EOF.  Drop the prompt
        lines (they start with '<user>@') and carriage returns so the
        caller sees just what the command printed.
        """
        pipe = f"echo {shlex.quote(cmd)} | clixon_cli -f /etc/clixon.xml"
        _, out = self.ssh(pipe)
        lines = [ln for ln in out.replace("\r", "").splitlines()
                 if not ln.startswith(f"{USER}@")]
        return "\n".join(lines)

    @contextmanager
    def step(self, desc):
        """One test step, reported as a TAP line: 'ok N - desc' on success,
        'not ok N - desc' on failure.  The step descriptions read as the
        manual procedure for the test."""
        self.nsteps += 1
        n = self.nsteps
        try:
            yield
        except BaseException:
            print(f"not ok {n} - {desc}", flush=True)
            raise
        print(f"ok {n} - {desc}", flush=True)

    def plan(self):
        """Emit the TAP plan line for the steps run so far."""
        print(f"1..{self.nsteps}", flush=True)


def until(fn, attempts=10, interval=1):
    """Retry fn() until it returns truthy, then return that value.

    Swallows exceptions from fn() between tries -- a check that reads a
    not-yet-settled DUT can raise or return false and simply be retried.
    Re-raises the last exception (or a TimeoutError) once attempts run out.
    """
    last = None
    for _ in range(attempts):
        try:
            result = fn()
        except Exception as e:
            last = e
            result = False
        if result:
            return result
        time.sleep(interval)
    if last:
        raise last
    raise TimeoutError(f"condition not met after {attempts} attempts")


def run(dut, test, name):
    """Run one test(dut) against `dut`, framed as TAP; returns 0 on pass.

    Emits '# <name>' bracketed by start/end timestamps (so a log reader
    can derive execution time), the steps' ok/not ok lines, and a '1..N'
    plan.
    """
    start = datetime.now()
    print(f"# {name}", flush=True)
    print(f"# started at {start.isoformat(timespec='seconds')}", flush=True)
    dut.nsteps = 0
    rc = 0
    try:
        test(dut)
    except Exception:
        rc = 1
        traceback.print_exc()
    dut.plan()
    end = datetime.now()
    print(f"# ended at {end.isoformat(timespec='seconds')} "
          f"({(end - start).total_seconds():.1f}s)", flush=True)
    return rc


def main(test):
    """Run one test(dut) as a standalone case and report; returns exit code.

    Attaches to the running DUT inside 'make test-sh', else boots a
    throwaway one just for this case.
    """
    name = os.path.basename(sys.argv[0]) or "test"
    try:
        with Dut() as dut:
            return run(dut, test, name)
    except Exception:
        traceback.print_exc()
        return 1


def case(test):
    """Decorator for a case's test(dut).  Running the file directly executes
    the test (attaching inside test-sh, else booting a throwaway); under the
    runner it just exposes test(dut)."""
    if test.__globals__.get("__name__") == "__main__":
        raise SystemExit(main(test))
    return test
