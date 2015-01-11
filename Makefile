# Top level Makefile
#
# Copyright (c) 2013-2014  Henrik Nordström <henrik@henriknordstrom.net>
# Copyright (c) 2014-2015  Joachim Nilsson <troglobit@gmail.com
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
.PHONY: run staging kernel packages clean distclean

ARCH            = arm
CROSS          ?= arm-linux-gnueabi-
CROSS_COMPILE  ?= $(CROSS)
KERNEL_VERSION  = 3.18.2

NAME           := "TroglOS Linux"
RELEASE_ID     := "chaos"
RELEASE        := "Chaos Devel `date --iso-8601`"
VERSION_ID     := "1.0-beta2"
VERSION        := "$(VERSION_ID), $(RELEASE)"
ID             := "troglos"
PRETTY_NAME    := "$(NAME) $(VERSION_ID)"
HOME_URL       := "http://troglobit.com"
SUPPORT_URL    := "https://github.com/troglobit/troglos"
BUG_REPORT_URL := "https://github.com/troglobit/troglos/issues"

ROOTDIR        := $(shell pwd)
STAGING         = $(ROOTDIR)/staging
STAGING_DIRS    = mnt proc sys bin sbin tmp
IMAGEDIR        = $(ROOTDIR)/images
MAKEFLAGS       = --silent --no-print-directory
DOWNLOADS      ?= $(shell xdg-user-dir DOWNLOAD 2>/dev/null || echo "$(ROOTDIR)/downloads")

export ARCH CROSS CROSS_COMPILE
export KERNEL_VERSION
export NAME VERSION_ID VERSION ID PRETTY_NAME HOME_URL
export ROOTDIR STAGING IMAGEDIR DOWNLOADS

all: staging kernel packages ramdisk

# qemu-img create -f qcow hda.img 2G
# +=> -hda hda.img
run:
	@echo "  QEMU    Starting $(NAME) ... (Use Ctrl-a c quit to exit Qemu)"
	@qemu-system-arm -nographic -m 128M -M versatilepb -usb					\
			 -device rtl8139,netdev=nic0						\
			 -netdev bridge,id=nic0,br=virbr0,helper=/usr/lib/qemu-bridge-helper	\
			 -kernel $(IMAGEDIR)/zImage        					\
			 -initrd $(IMAGEDIR)/initramfs.gz  					\
			 -append "root=/dev/ram console=ttyAMA0,115200 quiet"

staging:
	@echo "  STAGING Root file system ..."
	@mkdir -p $(IMAGEDIR)
	@mkdir -p $(STAGING)
	@for dir in $(STAGING_DIRS); do   \
		mkdir -p $(STAGING)/$$dir; \
	done
	-@(cd $(STAGING) && ln -sf . usr 2>/dev/null)
	@cp -a $(ROOTDIR)/initramfs/* $(STAGING)/
	@echo "NAME=\"$(NAME)\""                       > $(STAGING)/etc/os-release
	@echo "VERSION=\"$(VERSION)\""                 >>$(STAGING)/etc/os-release
	@echo "ID=\"$(ID)\""                           >>$(STAGING)/etc/os-release
	@echo "PRETTY_NAME=\"$(PRETTY_NAME)\""         >>$(STAGING)/etc/os-release
	@echo "VERSION_ID=\"$(VERSION_ID)\""           >>$(STAGING)/etc/os-release
	@echo "HOME_URL=\"$(HOME_URL)\""               >>$(STAGING)/etc/os-release
	@echo "SUPPORT_URL=\"$(SUPPORT_URL)\""         >>$(STAGING)/etc/os-release
	@echo "BUG_REPORT_URL=\"$(BUG_REPORT_URL)\""   >>$(STAGING)/etc/os-release
	@echo "$(VERSION)"                             > $(STAGING)/etc/version
	@echo "$(NAME) $(VERSION_ID) \\\\n \\l"        > $(STAGING)/etc/issue
	@echo "$(NAME) $(VERSION_ID)"                  > $(STAGING)/etc/issue.net
	@echo "$(RELEASE_ID)"                          > $(STAGING)/etc/hostname
	@sed -i 's/HOSTNAME/$(RELEASE_ID)/' $(STAGING)/etc/hosts

kernel:
	@$(MAKE) -j5 -C kernel all

kernel_menuconfig:
	@$(MAKE) -C kernel menuconfig

kernel_oldconfig:
	@$(MAKE) -C kernel oldconfig

kernel_saveconfig:
	@$(MAKE) -C kernel saveconfig

packages:
	@$(MAKE) -j5 -C packages all
	@$(MAKE) -j5 -C packages install

ramdisk:
	@echo "  INITRD  $(KERNEL_VERSION)"
	@$(MAKE) -f ramdisk.mk $@

PACK_EXCLUDES=Makefile
PACKLIST=$(filter-out $(PACK_EXCLUDES), $(notdir $(wildcard $(ROOTDIR)/packages/*)))
$(addsuffix -build,$(PACKLIST)): staging
	@$(MAKE) $(MFLAGS) -C $(ROOTDIR)/packages/ $@

$(addsuffix -install,$(PACKLIST)):
	@$(MAKE) $(MFLAGS) -C $(ROOTDIR)/packages/ $@

$(addsuffix -clean,$(PACKLIST)):
	@$(MAKE) $(MFLAGS) -C $(ROOTDIR)/packages/ $@

$(addsuffix -distclean,$(PACKLIST)):
	@$(MAKE) $(MFLAGS) -C $(ROOTDIR)/packages/ $@

$(addsuffix -menuconfig,$(PACKLIST)):
	@$(MAKE) -C $(ROOTDIR)/packages/ $@

$(addsuffix -saveconfig,$(PACKLIST)):
	@$(MAKE) -C $(ROOTDIR)/packages/ $@

clean:
	@for dir in kernel packages; do	\
		echo "  CLEAN   $$dir"; \
		/bin/echo -ne "\033]0;$(PWD) $$dir\007"; \
		$(MAKE) -C $$dir $@;	\
	done

distclean:
	@for dir in kernel packages; do	\
		echo "  REMOVE  $$dir"; \
		/bin/echo -ne "\033]0;$(PWD) $$dir\007"; \
		$(MAKE) -C $$dir $@;	\
	done
	-@$(RM) -rf $(STAGING) $(IMAGEDIR)
