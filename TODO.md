TODO
----

* Add possibility to store /etc on an emulated MTD
  Add "block2mtd.block2mtd=/dev/sda,128ki" to kernel cmdline
  http://www.spinics.net/lists/kernel/msg1862852.html <-- May be needed?
* Move to [crosstool-NG](https://github.com/crosstool-ng/crosstool-ng)
  to be able to use newer GCC versions, better auto-install of shared
  libraries and become more independent of Debian/Ubuntu packages that
  are used today.
* Linux Kbuild support for configuration management, more
* Targets than ARM and Versatile, with different packages and flavours
* More packages (add Finit and uftpd as a GitHub submodules!)
* U-Boot and Bareboot images, incl. Westermo squasfs `/boot` extension
* Add support for [Rocker](https://github.com/scottfeldman/qemu-rocker)
* Upgrade kernel to support both Rocker and swdev ...
* Add Quagga support
* Integrate the cool little CLI idea ...
* Little bit more documentation so people can get about easier

