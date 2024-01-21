Nanopi R2s
==========

This file documents the support for the Nanopi R2s by FriendlyELEC,
<https://wiki.friendlyelec.com/wiki/index.php/NanoPi_R2S>.


Features
--------

The basic support in Buildroot has been extended with a few critical
features:

  - Auto-probing of network driver modules at boot
  - DHCP server enabled on LAN port, 192.168.0.0/24, dynamic range
    from 192.168.0.100-.250, router itself is on 192.168.0.1
  - NTP and DNS server enabled on LAN port, handed out by DHCP server
  - SSH login (Dropbear) without root password allowed on LAN port
  - DHCP client on WAN port
  - nft rules for router set up in `/etc/nftables/firewall.nft`,
    default setup with IP masquerading from LAN to WAN
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
