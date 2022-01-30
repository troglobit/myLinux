LicheePi Nano
=============

This default configuration allows you to quickly get up and running with
the ultra-low-cost LicheePi Nano board.

The defconfig and the files in this board directory is all you need to
use Buildroot to build a bootable SD card image.  A flash image for the
built-in SPI flash is possible but not handled yet.  See the following
URL for some help in getting started porting it to this project:

  - <https://github.com/florpor/licheepi-nano>

This effort borrows heavily from Nick Matansev's (@unframework) similar
project.  The level of integration here is slightly higher, but for the
full support, I recommend his project:

  - <https://github.com/unframework/licheepi-nano-buildroot>

Nick's work, in turn, builds on the work done by the FunKey Zero project
which targets LicheePi Zero, a sibling board to the Nano, adapted for
the Nano:

   - <https://github.com/Squonk42/buildroot-licheepi-zero/>


Building
--------

    $ make licheepi_nano_defconfig
    $ make

The build may take 1.5 hours on a decent machine, or longer.  All build
artifacts are located in `output/images`.  Check that a `sdcard.img`
file has been created, it can now be written to the bootable SD card.

Example command to write image to SD card on Linux host:

    $ sudo dd if=output/images/sdcard.img of=/dev/mmcblk0 bs=1M
    $ sync

Linux and U-Boot
----------------

The build use a custom kernel and U-Boot by Nick:

  - <https://github.com/unframework/linux/commits/nano-5.11>
  - <https://github.com/unframework/u-boot/commits/2021.01-f1c100s>

The kernel changes have been cherry-picked from the @Lichee-Pi Linux
repo, with some minor additions for newer kernels:

  - [nano-5.2-tf branch](https://github.com/torvalds/linux/compare/master...Lichee-Pi:nano-5.2-tf)
  - [nano-5.2-flash branch](https://github.com/torvalds/linux/compare/master...Lichee-Pi:nano-5.2-flash)

The U-Boot is based on v2021.01d with changes from the original
@Lichee-Pi v2018.01 fork:

  - <https://github.com/Lichee-Pi/u-boot/commits/nano-v2018.01>, which
    is actually based on a branch by @Icenowy
  - <https://github.com/u-boot/u-boot/compare/master...Icenowy:f1c100s-spiflash>

Included is a customized DTS file that supports a 480x272 TFT screen for
the 40-pin flex-PCB connector on the board).  The source kernel comes
with a pre-existing DTS file, with support for 800x480 TFT resolution.
To change device tree, in the `make menuconfig` for Buildroot, under the
Kernel settings, change to use "in-tree" `suniv-f1c100s-licheepi-nano`
DTS file, then update `boot.cmd` and `genimage.cfg` to reference that
device tree as well.

To rebuild and activate the changes, without having to distclean:

    make host-uboot-tools-rebuild all

