# Top level Makefile
#
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
.PHONY: run staging kernel lib packages clean distclean

ARCH            = arm
HOST            = arm-linux-gnueabi
CROSS          ?= $(HOST)-
CROSS_COMPILE  ?= $(CROSS)
#KERNEL_RC       = -rc7
KERNEL_VERSION  = 4.3
# Add --debug to cmdline to debug finit
KERNEL_CMDLINE  = root=/dev/ram mem=128M console=ttyAMA0,115200 quiet init=/sbin/finit

CC              = $(CROSS_COMPILE)gcc
CFLAGS          =
CPPFLAGS        = -I$(STAGING)/include
LDLIBS          =
LDFLAGS         = -L$(STAGING)/lib

NAME           := TroglOS Linux
RELEASE_ID     := chaos
RELEASE         = Chaos Devel `date --iso-8601`
VERSION_ID     := 1.0-beta6
VERSION        := $(VERSION_ID), $(RELEASE)
ID             := "troglos"
PRETTY_NAME    := $(NAME) $(VERSION_ID)
HOME_URL       := http://troglobit.com
SUPPORT_URL    := https://github.com/troglobit/troglos
BUG_REPORT_URL := https://github.com/troglobit/troglos/issues

ROOTDIR        := $(shell pwd)
STAGING         = $(ROOTDIR)/staging
# usr/lib usr/share usr/bin usr/sbin 
STAGING_DIRS    = mnt proc sys lib share bin sbin tmp var home
IMAGEDIR        = $(ROOTDIR)/images
PERSISTENT     ?= $(shell xdg-user-dir DOCUMENTS)/TroglOS
MTD            ?= $(PERSISTENT)/Config.mtd
MAKEFLAGS       = --silent --no-print-directory
DOWNLOADS      ?= $(shell xdg-user-dir DOWNLOAD)

export ARCH HOST CROSS CROSS_COMPILE
export KERNEL_VERSION KERNEL_RC
export CC CFLAGS CPPFLAGS LDLIBS LDFLAGS
export NAME VERSION_ID VERSION ID PRETTY_NAME HOME_URL
export ROOTDIR STAGING IMAGEDIR DOWNLOADS

all: staging kernel lib packages ramdisk

# qemu-img create -f qcow hda.img 2G
# +=> -hda hda.img
#			 -drive file=disk.img,if=virtio
#			 -dtb $(IMAGEDIR)/versatile-ab.dtb
#			 -dtb $(IMAGEDIR)/versatile-pb.dtb
run:
	@echo "  QEMU    Ctrl-a x -- exit | Ctrl-a c -- switch console/monitor"
	-@mkdir -p $(PERSISTENT) 2>/dev/null
	-@[ ! -e $(MTD) ] && dd if=/dev/zero of=$(MTD) bs=16384 count=960
	@qemu-system-arm -nographic -m 128M -M versatilepb -usb						\
			 -drive if=scsi,file=$(MTD),format=raw						\
			 -device rtl8139,netdev=nic0							\
			 -netdev bridge,id=nic0,br=virbr0,helper=/usr/lib/qemu/qemu-bridge-helper	\
			 -kernel $(IMAGEDIR)/zImage        						\
			 -initrd $(IMAGEDIR)/initramfs.gz  						\
			 -append "$(KERNEL_CMDLINE) block2mtd.block2mtd=/dev/sda,,Config"

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
	@echo "  INSTALL Toolchain shared libraries ..."
	# XXX: UGLY FIXME!!
	@cp -a /usr/arm-linux-gnueabi/lib/* $(STAGING)/lib/

# Cleanup staging (may need to separate into a staging + romfs dir, like uClinux-dist)
ramdisk:
	@echo "  INITRD  $(NAME) $(KERNEL_VERSION)"
	@rm -rf $(STAGING)/lib/pkgconfig
	@rm -rf $(STAGING)/lib/*.a
	@rm -rf $(STAGING)/share/man
	@touch $(STAGING)/etc/version
	@$(MAKE) -f ramdisk.mk $@

kernel:
	@$(MAKE) -j5 -C kernel all

kernel_menuconfig:
	@$(MAKE) -C kernel menuconfig

kernel_oldconfig:
	@$(MAKE) -C kernel oldconfig

kernel_defconfig:
	@$(MAKE) -C kernel defconfig

kernel_saveconfig:
	@$(MAKE) -C kernel saveconfig

kernel_install:
	@$(MAKE) -C kernel dtbinst

# Packages may depend on libraries, so we build libs first
packages: lib

packages lib:
	@$(MAKE) -j5 -C $@ all
	@$(MAKE) -j5 -C $@ install

TARGETS=$(shell find packages -maxdepth 1 -mindepth 1 -type d)
include quick.mk

TARGETS=$(shell find lib -maxdepth 1 -mindepth 1 -type d)
include quick.mk

clean:
	@for dir in kernel lib packages; do			\
		echo "  CLEAN   $$dir";				\
		/bin/echo -ne "\033]0;$(PWD) $$dir\007";	\
		$(MAKE) -C $$dir $@;				\
	done

distclean:
	@for dir in kernel lib packages; do			\
		echo "  REMOVE  $$dir";				\
		/bin/echo -ne "\033]0;$(PWD) $$dir\007";	\
		$(MAKE) -C $$dir $@;				\
	done
	-@$(RM) -rf $(STAGING) $(IMAGEDIR)
