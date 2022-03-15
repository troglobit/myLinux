Marvell CN9130-CRB
==================

This default configuration allows you to get quickly up and running with
a full Buildroot system on the Marvell customer reference board for the
CN9130 customer reference board (CRB).  This board is possible to run in
multiple topologies (or configurations), the factory default is topo A,
often referred to as CRB-A.  Another is topology B, or CRB-B.  Where the
difference between is how the SoC serdes lanes are used, in Linux the
main difference is that topology A enables the SPI flash, and topology
B enables NAND flash.  Hence the two different boot menu alternatives,
which select different Linux kernel device trees.

The CN9130, also known as Octeon TX2, is the logical successor to the
Armada 8040.  On the CRB it comes bundled with an 88E6393X switchcore,
which provides 9 of the total 11 front Ethernet ports.  The other two
are directly connected to the SoC, providing 1.0 and 2.5 GBps ports.
The switchcore is connected to a 10 Gbps port on the SoC, and one of the
switchcore ports is an SFP slot capable of up to 10 Gbps.

Support for all board features, including the device trees, is available
from Linux mainline as of 5.17.  The following software components are
are used in this defconfig:

  - Linux v5.17 (mainline)
  - U-Boot v2022.01 (mainline)
  - ATF v2.5 (mainline)


Building
--------

    $ make cn9130_defconfig
    $ make

This generates the kernel image, compiled device trees, the rootfs as a
tar.gz, and a filesystem image for booting from SD card.

Flash the image to your SD card drive, your `of=` device may differ:

    $ sudo dd if=output/images/sdcard.img of=/dev/mmcblk0 bs=1M status=progress oflag=direct
    $ sync


Booting
-------

As mentioned previously, the CRB can be strapped to boot from different
sources.  See the CN91XX-CRB Hardware Reference Manual v1.3, or later,
for details on this.  Here we provide the necessary DIP switch settings
for booting the CRB-A topology from SD card, which should match the
factory defaults:

    SW1: 1011      (CPU/DDR frequency config only, safe to leave as-is)
    SW2: 011010

For interactive boot, i.e., to interact with the U-boot boot loader or
the Linux console, you need a UART connection, using the on-board micro
USB port set to 115200 8N1.

Modern U-Boot reads the extlinux.conf menu, from where you an select the
appropriate alternative, or wait 3 sec for the default to start.

The system then proceeds to load the kernel and bootstrap userspace,
concluding with presenting you with a login prompt.  The default user is
'root', no password.  Use the `password` tool after login to set one.


Networking
----------

To enable Ethernet networking, load the `mv88e6xxx` kernel module, and
bring up each respective interface needed:

    # modprobe mv88e6xxx
    # ifconfig p1 up

A more advanced scenario is setting up switching between the ports using
the Linux bridge.  The kernel switchdev layer, and DSA driver, ensure
switch functions are "offloaded" to the HW switch, i.e., all traffic
between LAN ports never reach the CPU.  For this you need the iproute2
suite of tools.
