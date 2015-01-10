TroglOS a Virtual Devboard
==========================

Build your own Linux from scratch with TroglOS *ARM Versatile* based
virtual devboard:

![TroglOS v1.0-beta1](http://ftp.troglobit.com/troglos/TroglOS-v1.0-beta1.png "Booting TroglOS Virtual Devboard")

TroglOS is a playful, but working, example of how to create a virtual
devboard, based on Linux and BusyBox.  You can use it for testing your
embedded applications before the actual hardware arrives.  It is also
useful for reference when said hardware starts acting up -- you know it
always does, right?

Requirements
------------

The build environment requires at least the following tools, tested on
Ubuntu 14.04:

* gcc-arm-linux-gnueabi
* curl
* make
* gcc
* quilt
* probably more, gzip?, mkimge?

Clone this repository, then type `make`.  When the build has completed,
start Qemu with `make run` -- Have fun!

Upgrading Linux
---------------

Change the `KERNEL_VERSION` in the top-level `Makefile`.  If the kernel
is just a minor patch release, you're done.

If it's a major kernel upgrade, copy the latest `kernel/config-X.YY` to
`kernel/config-X.ZZ` and call `make kernel_oldconfig`.  This will unpack
the kernel and give you a set of questions for all new features.

Make sure to do a `make kernel_saveconfig`, and possibly add the new
`kernel/config-X.ZZ` to GIT.

