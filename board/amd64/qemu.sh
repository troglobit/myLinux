#!/bin/sh

ARGS="rootwait root=/dev/vda console=tty1 console=ttyS0"
ARGS="$ARGS quiet"
#ARGS="$ARGS loglevel=7 panic=10 debug ignore_loglevel"
ARGS="$ARGS net.ifnames=0 systemd.show_status=1"

# Disable pulseaudio warning
export QEMU_AUDIO_DRV=none

# Save current line settings, then disable all of them so that
# everything is passed through to the guest (C-c, C-z etc.).
line=$(stty -g)
stty raw

# Start Qemu -nographic or -display none
qemu-system-x86_64 -M q35,accel=kvm -smp 2 -nographic -m 256M -cpu host -enable-kvm	\
	-device i6300esb -rtc clock=host -kernel output/images/bzImage			\
	-drive file=output/images/rootfs.ext2,if=virtio,format=raw -append "$ARGS"	\
	-net nic,model=virtio -net user -net nic,model=virtio -net user

# Restore TTY from Qemu target
stty ${line}

