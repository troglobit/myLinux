#!/usr/bin/env python3
"""myLinux test runner.

    ./run.py               boot a DUT, run every test/case/**/*.py, report
    ./run.py case/...      run only the given case file(s)
    ./run.py --shell       boot a DUT and open an interactive test shell

Inside a --shell session the DUT stays up and is shared: run individual
cases (./case/system/hostname.py) or ./run.py against it with no reboot,
and ./console attaches to its serial console.
"""
import importlib.util
import os
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import my

HERE = os.path.dirname(os.path.abspath(__file__))
CASEDIR = os.path.join(HERE, "case")


def discover():
    found = []
    for root, _, files in os.walk(CASEDIR):
        for fn in sorted(files):
            if fn.endswith(".py") and not fn.startswith("_"):
                found.append(os.path.join(root, fn))
    return sorted(found)


def load(path):
    spec = importlib.util.spec_from_file_location("case", path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def run_cases(paths):
    fails = 0
    with my.Dut() as dut:
        if dut.console:
            print(f"# DUT up (console: {dut.console})")
        for path in paths:
            name = os.path.relpath(path, CASEDIR)
            if my.run(dut, load(path).test, name):
                fails += 1
    print(f"# {len(paths) - fails}/{len(paths)} cases passed")
    return 1 if fails else 0


def shell():
    with my.Dut() as dut:
        env = dict(os.environ,
                   MYTEST_ATTACH="1",
                   DUT_SSH_PORT=str(my.SSH_PORT),
                   PYTHONPATH=HERE)          # so cases can 'import my'
        if dut.consock:
            env["MYTEST_CONSOLE"] = dut.consock

        print(f"DUT up (console log: {dut.console})")
        print("  run a case : ./case/system/hostname.py")
        print("  run all    : ./run.py")
        print("  console    : ./console          (Ctrl-] to detach)")
        print(f"  ssh        : ssh -p {my.SSH_PORT} admin@localhost")
        print("Exit this shell to shut the DUT down.")

        sh = os.environ.get("SHELL", "/bin/sh")
        prompt = "mylinux-test"
        rcfile = None
        if os.path.basename(sh) == "bash":
            # bash reloads PS1 from ~/.bashrc, so hand it an rcfile that
            # sources the user's config and then sets our prompt.
            rc = tempfile.NamedTemporaryFile("w", suffix=".bashrc", delete=False)
            rc.write("[ -f ~/.bashrc ] && . ~/.bashrc\n")
            # ~/.bashrc often drives the prompt from PROMPT_COMMAND, which
            # would re-clobber PS1 on every prompt -- drop it so ours sticks.
            rc.write("unset PROMPT_COMMAND\n")
            rc.write(f'PS1="\\[\\e[1;33m\\]{prompt}\\[\\e[0m\\]:\\w\\$ "\n')
            rc.close()
            rcfile = rc.name
            cmd = [sh, "--rcfile", rcfile, "-i"]
        else:
            env["PS1"] = f"[{prompt}] \\$ "   # sh inherits PS1
            cmd = [sh, "-i"]

        try:
            subprocess.call(cmd, cwd=HERE, env=env)
        finally:
            if rcfile:
                os.unlink(rcfile)
    return 0


def main(argv):
    if "--shell" in argv:
        return shell()

    args = [a for a in argv if not a.startswith("-")]
    paths = [os.path.abspath(a) for a in args] if args else discover()
    if not paths:
        print(f"No test cases under {CASEDIR}")
        return 1

    if not os.environ.get("MYTEST_ATTACH"):
        print(f"Booting DUT for {len(paths)} test case(s) ...")
    return run_cases(paths)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
