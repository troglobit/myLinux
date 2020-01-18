.EXPORT_ALL_VARIABLES:

KERNEL_IMG    := zImage
QEMU_ARCH     := ppc64
QEMU_MACH     := ppce500
QEMU_NIC      := rtl8139
QEMU_SCSI     := virtio-scsi-pci
QEMU_9P       := virtio-9p-pci
QEMU_EXTRA    := -cpu e5500
QEMU_DTB      :=
