SolidRun Clearfog
=================

This file documents the myLinux defconfig for the Clearfog by Solid Run.
Buildroot support the ClearFog Base, but myLinux support the Pro which
comes with a built-in Marvell 88E6176 Linkstreet switch.

Vendor docs: <https://developer.solid-run.com/article-tags/clearfog/>

See the Buildroot readme.txt for more information.


Features
--------

  - Switch enabled by default as `br0` (non-VLAN filtering)
  - DHCP server enabled on LAN ports, 192.168.0.0/24, router on .1
  - NTP and DNS server enabled on LAN ports, handed out by DHCP server
  - SSH login (Dropbear) without root password allowed on LAN ports
  - DHCP client on WAN port
  - nft rules for router set up in `/etc/nftables/firewall.nft`
  - Several Realtek WiFi drivers + firmware for USB port dongles
  - Auto-start of `hostapd` on `wlan0` if a WiFi dongle is found
  - Auto-add `wlan0` interface to `br0` when AP mode is activated


Configuration
-------------

The defaults should be suitable for most home gateway use.  To change
the setup you can log in using SSH:

    ssh root@192.168.0.1

> **Note:** please change root password after first login.

The `mg` editor and BusyBox `vi` are available.  The former is more
user-friendly for beginners.

Interesting files to edit:

  - `/etc/dnsmasq.conf`
  - `/etc/hostapd.conf`
  - `/etc/nftables/firewall.nft`

Either reboot to activate your changes, or restart the service using,
e.g.:

    initctl restart dnsmasq

The firewall can be reloaded using:

    /etc/nftables/firewall.nft


Build
-----

The build process is the same as the default Buildroot, only the name of
the defconfig differs:

    make clearfog_defconfig

Build all components:

    make

The results of the build are available in `./output/images/`


Booting
-------

> **WARNING!** The dd command below overwrites all of
> `/dev/<your-microsd-device>` -- use with care!

To determine the device associated to the SD card have a look in the
`/proc/partitions` file:

    cat /proc/partitions

Buildroot prepares a bootable `sdcard.img` image in the `output/images/`
directory, ready to be dumped on a microSD card.  Launch the following
command as root:

    dd if=output/images/sdcard.img of=/dev/<your-microsd-device> conv=fdatasync

Example:

    sudo dd if=output/images/sdcard.img of=/dev/mmcblk0 bs=1M status=progress oflag=direct


[readme.txt]: https://gitlab.com/buildroot.org/buildroot/-/blob/master/board/solidrun/clearfog/readme.txt?ref_type=heads
