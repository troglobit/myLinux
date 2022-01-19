setenv fdt_high ffffffff

setenv bootargs console=ttyS0,115200 root=/dev/mmcblk0p2 rootwait quiet

fatload mmc 0 $kernel_addr_r zImage
fatload mmc 0 $fdt_addr_r sun8i-h3-nanopi-neo.dtb

bootz $kernel_addr_r - $fdt_addr_r
