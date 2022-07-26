FriendlyARM NanoPi NEO
======================

This build provides fully functioning myLinux system for the NEO, with
both wired and wireless communication available.

It is possible to log in both using the serial/debug port or SSH for the
initial board configuration.  See below for details.

Having completed the below steps, insert the micro SDcard in your NEO
and power it up.  The console is on the serial line, 115200 8N1.

> Default username/password: `root/secret` -- CHANGE AT FIRST LOGIN!


How to Build
------------

Configure:

    $ make neo_defconfig

Compile everything and build the SD card image:

    $ make

All build artifacts are located in the `output/images/` directory.


How to write the SD card
------------------------

Once the build is finished you will have an `sdcard.img` image in the in
the `output/images/` directory.

Flash rootfs image to sdcard drive, your `of=` device may differ:

    $ sudo dd if=output/images/sdcard.img of=/dev/mmcblk0 bs=1M

> **Note:** replace `mmcblk0` with the actual device with your SDcard!
