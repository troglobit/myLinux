import gdb
import os
import time
from select import select
from subprocess import Popen, PIPE

class TargetConsole (gdb.Command):
    """ Connect to a remote system with KGDB support over a conserver console """

    def __init__ (self):
        super (TargetConsole, self).__init__ ("target-console", gdb.COMMAND_USER)

    def usage():
        print("Connect to kgdb over a conserver console\n" +
              "target-console <CONSOLE-NAME>\n")

    def invoke (self, args, from_tty):
        parsed = [gdb.parse_and_eval(arg) for arg in args.split()]
        if len(parsed) != 1:
            return self.usage()

        con = parsed[0].string()

        p = Popen(args = ["console", "-f", con],
                  stdin = PIPE, stdout = PIPE, stderr = PIPE)

        p.stdin.write(b"echo ttyS0 >/sys/module/kgdboc/parameters/kgdboc\r\n")
        p.stdin.write(b"echo g >/proc/sysrq-trigger\r\n")
        p.stdin.flush()

        out = b""
        while True:
            r, w, x = select((p.stdout.fileno(),), (), (), .5)
            if not len(r):
                break

            out += os.read(p.stdout.fileno(), 4096)

        if b"Entering KGDB" not in out:
            print("Unable to enter KGDB.\n" +
                  "The target must be at the shell prompt, running a kernel" +
                  "with KGDB support.")

        p.terminate()
        p.wait()

        os.system("socat pty,link=/tmp/wmo-kgdb,wait-slave exec:'console -f " +
                  con + "' &")
        time.sleep(.5)
        gdb.execute("target remote /tmp/wmo-kgdb")

TargetConsole ()
