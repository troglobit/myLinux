.EXPORT_ALL_VARIABLES:

CROSS_COMPILE := x86_64-unknown-linux-gnu-
KERNEL_IMG    := bzImage
QEMU_ARCH     := x86_64
QEMU_MACH     := q35
QEMU_NIC      := rtl8139
QEMU_SCSI     := virtio-scsi-device
QEMU_9P       := virtio-9p-device
QEMU_EXTRA    := -cpu host -enable-kvm -rtc clock=host
QEMU_DTB      :=

