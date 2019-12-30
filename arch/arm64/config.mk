.EXPORT_ALL_VARIABLES:

KERNEL_IMG    := Image
QEMU_ARCH     := aarch64
# vexpress-a15
QEMU_MACH     := virt
# lan9118
QEMU_NIC      := rtl8139
QEMU_SCSI     := virtio-scsi-device
QEMU_9P       := virtio-9p-device
QEMU_EXTRA    := -cpu cortex-a57 -smp 2
# freescale/fsl-ls2080a-simu.dtb # freescale/fsl-ls2080a-rdb.dtb
#QEMU_DTB      := arm/vexpress-v2f-1xv7-ca53x2.dtb

