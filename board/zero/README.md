OrangePi Zero
=============

This build provides fully functioning myLinux system for the Zero, with
both wired and wireless communication available.

It is possible to log in both using the serial/debug port or SSH for the
initial board configuration.  See below for details.

> Default username/password: `root/secret` -- CHANGE AT FIRST LOGIN!


First Boot
----------

Having compelted the steps below, here are the recommended steps to
perform at first boot:

  1. Plug the device into the wired network
  2. The device uses DHCP to acquire an address
  3. Find the device using, e.g., `mdns-scan` on Linux:

        $ mdns-scan
        + anarchy._ssh._tcp.local
        ...

  4. Ping the device `ping anarchy.local`
  5. Log in using ssh (accept the host key):

        $ ssh root@anarchy.local
        root@anarchy.local's password:
                         ___    __
         .--------.--.--|   |  |__.-----.--.--.--.--.  (@-
         |        |  |  |.  |  |  |     |  |  |_   _|  //\  :: Troglobit Software
         |__|__|__|___  |.  |__|__|__|__|_____|__.__|  V_/_ :: https://troglobit.com
                          |_____|:  1   |
                        |::.. . | A N A R C H Y rel.
                        `-------'
        root@anarchy:~#

  6. Change your system password, use `pwgen` for inspiration:

        root@anarchy:~# passwd

  7. Change hostname, skip if you don't need it:

        root@anarchy:~# edit /etc/hostname
        root@anarchy:~# edit /etc/hosts

     Changes take effect on next reboot, or: `hostname /etc/hostname`

  9. Configure your WiFi network, the sample config should be helpful:

        root@anarchy:~# edit /etc/wpa_supplicant.conf

  9. Enable the wpa_supplicant Finit service:

        root@anarchy:~# initctl enable wpa_supplicant
        root@anarchy:~# initctl reload

     Check status of service with: `initctl status wpa_supplicant`

  10. Done


How to Build
------------

Configure:

    $ make zero_defconfig

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
