# Handles Qemu target (run), image creation, and other arch specifics
#
# Copyright (c) 2014-2020  Joachim Nilsson <troglobit@gmail.com>
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
.PHONY: run image sdcard

# Source configured settings for this arch
include $(ROOTDIR)/system/core.mk

run:
	@echo "  QEMU    Ctrl-a x -- exit | Ctrl-a c -- toggle console/monitor"
	-@mkdir -p $(QEMU_HOST) 2>/dev/null
	-@[ ! -e $(QEMU_MNT) ] && dd if=/dev/zero of=$(QEMU_MNT) bs=16384 count=960
	@qemu $(IMAGEDIR)/$(KERNEL_IMG) $(IMAGEDIR)/troglos.img

image:
	@echo "  SQUASH  Creating TroglOS SquasFS image"
	@mksquashfs $(ROMFS)/* $(IMAGEDIR)/troglos.img -noappend -no-xattrs -comp xz -all-root -b 128k

sdcard:
	@$(MAKE) -C $(ARCH)/$(MACH) $@
