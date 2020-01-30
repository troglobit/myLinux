.EXPORT_ALL_VARIABLES:

QEMU_ARCH     := arm
QEMU_MACH     := versatilepb
QEMU_NIC      := smc91c111
QEMU_SCSI     := virtio-scsi-pci
QEMU_EXTRA    := -rtc base=utc,clock=rt
QEMU_DTB      := versatile-pb.dtb

