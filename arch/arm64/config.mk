.EXPORT_ALL_VARIABLES:

CROSS_COMPILE := aarch64-unknown-linux-gnu-
KERNEL_IMG    := Image.gz
QEMU_ARCH     := aarch64
# vexpress-a15
QEMU_MACH     := virt
# lan9118
QEMU_NIC      := rtl8139
QEMU_EXTRA    := -cpu cortex-a57
# freescale/fsl-ls2080a-simu.dtb # freescale/fsl-ls2080a-rdb.dtb
#QEMU_DTB      := arm/vexpress-v2f-1xv7-ca53x2.dtb

