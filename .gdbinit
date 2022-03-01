# Collection of useful macros to aid debugging with QEMU
#
# This file will be automatically loaded when launching GDB from
# staging/.
#
# In addition, if there is a file called .gdbinit in your home
# directory, that file is also loaded. Please put personal output
# settings etc. there, not in this file.
#
# When doing post-mortem debugging (core dumps), just launch gdb, from
# the staging directory, with the binary and core file as the first
# and second argument respectively.
#
# Examples:
#
#   To run a user application under gdb:
#     user-load sbin/ip
#     # set relevant breakpoints and so on
#     run link set dev eth0 master br0
#
#   To attach to an already running process:
#     user-attach sbin/finit 1
#     # set relevant breakpoints and so on
#     continue
#
#   To debug the kernel:
#     kernel-attach
#     # set relevant breakpoints and so on
#     continue
#
#   To debug the kernel on a physical target:
#     # login to device and enter the shell
#     kernel-attach-console ser1
#     # set relevant breakpoints and so on
#     continue
#
#   To debug a core dump:
#     user@host:~/weos/staging # gdb bin/sleep ~/tmp/sleep.core
#
source .gdbinit.py

# Connect to KGDB over a conserver console and load the kernel debug symbols.
#
# The remote system must be running a kernel built with KGDB
# support. This command also assumes that the system is logged in to
# and broken out into the shell.
#
# If the system ever gets stuck in KGDB, type the following string
# into the console window to detach from KGDB: $D#44$
#
# usage: kernel-attach-console <CONSOLE>
# example: kernel-attach-console ser1
define kernel-attach-console
  file boot/vmlinux
  target-console $arg0
end

# Connect to the GDB Server in QEMU and load the kernel debug symbols.
#
# usage: kernel-attach [HOST] [PORT]
define kernel-attach
  file boot/vmlinux
  if $argc == 0
    target extended-remote localhost:4711
  end
  if $argc == 1
    target extended-remote localhost:$arg0
  end
  if $argc == 2
    target extended-remote $arg0:$arg1
  end
end

# Connect to a GDB Server process running in the guest OS.
#
# usage: user-connect [HOST] [PORT]
define user-connect
  if $argc == 0
    target extended-remote localhost:4712
  end
  if $argc == 1
    target extended-remote localhost:$arg0
  end
  if $argc == 2
    target extended-remote $arg0:$arg1
  end
  set sysroot ./
end

# Select an application to debug in the guest OS.
#
# usage: user-load [HOST] [PORT] <FILE>
# example: user-load sbin/finit
define user-load
  if $argc == 1
    user-connect
    file $arg0
    set remote exec-file /$arg0
  end
  if $argc == 2
    user-connect $arg0
    file $arg1
    set remote exec-file /$arg1
  end
  if $argc == 3
    user-connect $arg0 $arg1
    file $arg2
    set remote exec-file /$arg2
  end
end

# Attach to a running process in the guest OS.
#
# usage: user-attach [HOST] [PORT] <FILE> <PID>
# example: user-attach sbin/finit 1
define user-attach
  if $argc == 2
    user-load $arg0
    attach $arg1
  end
  if $argc == 3
    user-load $arg0 $arg1
    attach $arg2
  end
  if $argc == 4
    user-load $arg0 $arg1 $arg2
    attach $arg3
  end
end

directory ../build

set solib-absolute-prefix .
set solib-search-path lib
