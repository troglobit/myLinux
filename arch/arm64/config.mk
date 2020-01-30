.EXPORT_ALL_VARIABLES:

QEMU_ARCH     := aarch64
QEMU_MACH     := virt
QEMU_NIC      := rtl8139
QEMU_SCSI     := virtio-scsi-device
QEMU_EXTRA    := -cpu cortex-a57 -smp 2
# freescale/fsl-ls2080a-simu.dtb # freescale/fsl-ls2080a-rdb.dtb
#QEMU_DTB      := arm/vexpress-v2f-1xv7-ca53x2.dtb

