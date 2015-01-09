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

The build environment requires at least the following tools, tested on
Ubuntu 14.04:

* gcc-arm-linux-gnueabi
* curl
* make
* gcc
* quilt
* probably more, gzip?, mkimge?

Clone this repository, then type `make`.  When the build has completed,
start Qemu with `make run`

Have fun!  :-)

TODO
----

* Linux Kbuild support for configuration management, more
* Targets than ARM and Versatile, with different packages and flavours
* More packages (add Finit and uftpd as a GitHub submodules!)
* U-Boot and Bareboot images, incl. Westermo squasfs `/boot` extension
* Add support for [Rocker](https://github.com/scottfeldman/qemu-rocker)
* Upgrade kernel to support both Rocker and swdev ...
* Add Quagga support
* Integrate the cool little CLI idea ...
* Little bit more documentation so people can get about easier

  -- Joachim 

