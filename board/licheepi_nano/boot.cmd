setenv bootargs console=tty0 console=ttyS0,115200 root=/dev/mmcblk0p2 rootwait rw quiet
load mmc 0:1 0x80C00000 suniv-f1c100s-licheepi-nano.dtb
load mmc 0:1 0x80008000 zImage
bootz 0x80008000 - 0x80C00000
