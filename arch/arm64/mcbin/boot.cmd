mmc dev 1
fatload mmc 1:1 $kernel_addr  Image
fatload mmc 1:1 $fdt_addr     armada-8040-mcbin.dtb
fatload mmc 1:1 $initrd_addr initramfs.img
setenv bootargs console=ttyS0,115200 earlyprintk root=/dev/root rootwait panic=30
booti $kernel_addr $initrd_addr:$filesize $fdt_addr
