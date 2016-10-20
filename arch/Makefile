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

MTD               ?= $(PERSISTENT)/Config.mtd
KERNEL_CMDLINE     = $(shell echo $(CONFIG_LINUX_CMDLINE) | sed 's/"//g')

# qemu-img create -f qcow hda.img 2G
# +=> -hda hda.img
#			 -drive file=disk.img,if=virtio
#			 -dtb $(IMAGEDIR)/versatile-ab.dtb
#			 -dtb $(IMAGEDIR)/versatile-pb.dtb
run:
	@echo "  QEMU    Ctrl-a x -- exit | Ctrl-a c -- switch console/monitor" | tee -a $(BUILDLOG)
	-@mkdir -p $(PERSISTENT) 2>/dev/null
	-@[ ! -e $(MTD) ] && dd if=/dev/zero of=$(MTD) bs=16384 count=960
	@QEMU_AUDIO_DRV=none qemu-system-arm -nographic -m 128M -M versatilepb -usb			\
			 -drive if=scsi,file=$(MTD),format=raw						\
			 -device rtl8139,netdev=nic0							\
			 -netdev bridge,id=nic0,br=virbr0,helper=/usr/lib/qemu/qemu-bridge-helper	\
			 -kernel $(IMAGEDIR)/zImage        						\
			 -initrd $(IMAGEDIR)/initramfs.gz  						\
			 -append "$(KERNEL_CMDLINE) block2mtd.block2mtd=/dev/sda,,Config"
