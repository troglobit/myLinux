# Handles Qemu target (run), image creation, and other arch specifics
#
# Copyright (c) 2014-2016  Joachim Nilsson <troglobit@gmail.com>
#
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted, provided that the
# above copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
# IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
.PHONY: run

# Source configured settings for this arch
include $(CONFIG)

KERNEL_CMDLINE     = $(shell echo $(CONFIG_LINUX_CMDLINE) | sed 's/"//g')

# /mnt
#  File on host to mount under /mnt in guest
QEMU_MTD       ?= $(PERSISTENT)/Config.mtd
QEMU_MTD_ARGS  := -drive if=scsi,file=$(QEMU_MTD),format=raw
KERNEL_CMDLINE += block2mtd.block2mtd=/dev/sda,,Config

# /host
#  Directory on host to mount under /host in guest
QEMU_HOSTFS      ?= $(PERSISTENT)
QEMU_HOSTFS_ARGS := -device virtio-9p-pci,fsdev=hostfs,mount_tag=hostfs
QEMU_HOSTFS_ARGS += -fsdev local,id=hostfs,path=$(QEMU_HOSTFS),security_model=none

# Network
#  bridge: Bridge the quests interface to the host's virbr0 (default)
#  tap   : Guest interface is exported as exported to host as qtap
QEMU_NET      ?= bridge
QEMU_NET_ARGS := -device rtl8139,netdev=nic0
ifeq ($(QEMU_NET),bridge)
QEMU_NET_ARGS += -netdev bridge,id=nic0,br=virbr0,helper=/usr/lib/qemu/qemu-bridge-helper
endif
ifeq ($(QEMU_NET),tap)
QEMU_NET_ARGS += -netdev tap,id=nic0,ifname=qtap,script=$(ROOTDIR)/bin/qtap-up.sh
endif

run:
	@echo "  QEMU    Ctrl-a x -- exit | Ctrl-a c -- switch console/monitor" | tee -a $(BUILDLOG)
	-@mkdir -p $(PERSISTENT) 2>/dev/null
	-@[ ! -e $(QEMU_MTD) ] && dd if=/dev/zero of=$(QEMU_MTD) bs=16384 count=960
	@QEMU_AUDIO_DRV=none qemu-system-arm -nographic -m 256M -M versatilepb -usb			\
			 $(QEMU_MTD_ARGS)								\
			 $(QEMU_HOSTFS_ARGS)								\
			 $(QEMU_NET_ARGS)								\
			 -kernel $(IMAGEDIR)/zImage        						\
			 -initrd $(IMAGEDIR)/initramfs.gz  						\
			 -dtb $(IMAGEDIR)/versatile-pb.dtb						\
			 -append "$(KERNEL_CMDLINE) quiet"
