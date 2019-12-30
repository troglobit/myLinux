.EXPORT_ALL_VARIABLES:

KERNEL_IMG    := bzImage
QEMU_ARCH     := x86_64
QEMU_MACH     := q35,accel=kvm
QEMU_NIC      := rtl8139
QEMU_SCSI     := virtio-scsi-pci
QEMU_EXTRA    := -cpu host -enable-kvm -rtc clock=host
QEMU_DTB      :=

