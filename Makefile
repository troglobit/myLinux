# Top level Makefile
#
# Copyright (c) 2014-2016  Joachim Nilsson <troglobit@gmail.com
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
CROSS          ?= arm-linux-gnueabi-
CROSS_COMPILE  ?= $(CROSS)
CROSS_TARGET    = $(CROSS_COMPILE:-=)

CC              = $(CROSS_COMPILE)gcc
CFLAGS          =
CPPFLAGS        = -I$(STAGING)/include
LDLIBS          =
LDFLAGS         = -L$(STAGING)/lib
STRIP           = $(CROSS_COMPILE)strip

OSNAME         := TroglOS Linux
OSRELEASE_ID   := chaos
OSRELEASE       = Chaos Devel `date --iso-8601`
OSVERSION_ID   := 1.0-rc1
OSVERSION      := $(OSVERSION_ID), $(OSRELEASE)
OSID           := "troglos"
OSPRETTY_NAME  := $(OSNAME) $(OSVERSION_ID)
OSHOME_URL     := http://troglobit.com
TROGLOHUB      := https://github.com/troglobit
SUPPORT_URL    := $(TROGLOHUB)/troglos
BUG_REPORT_URL := $(TROGLOHUB)/troglos/issues

ROOTDIR        := $(shell pwd)
PATH           := $(ROOTDIR)/bin:$(PATH)
CONFIG         := $(ROOTDIR)/.config
STAGING         = $(ROOTDIR)/staging
PKG_CONFIG_PATH:= $(STAGING)/lib/pkgconfig
BUILDLOG       := $(ROOTDIR)/build.log
# usr/lib usr/share usr/bin usr/sbin 
STAGING_DIRS    = mnt proc sys lib share bin sbin tmp var home
IMAGEDIR        = $(ROOTDIR)/images
PERSISTENT     ?= $(shell xdg-user-dir DOCUMENTS)/TroglOS
MTD            ?= $(PERSISTENT)/Config.mtd
DOWNLOADS      ?= $(shell xdg-user-dir DOWNLOAD)

ifdef V
  ifeq ("$(origin V)", "command line")
    KBUILD_VERBOSE = $(V)
  endif
endif
ifndef KBUILD_VERBOSE
  KBUILD_VERBOSE = 0
endif
ifeq ($(KBUILD_VERBOSE),1)
MAKEFLAGS       =
REDIRECT        = 2>&1 | teepee $(BUILDLOG)
else
MAKEFLAGS       = --silent --no-print-directory
REDIRECT        = >> $(BUILDLOG) 2>&1
endif

export ARCH BUILDLOG CROSS CROSS_COMPILE CROSS_TARGET
export CC CFLAGS CPPFLAGS LDLIBS LDFLAGS STRIP
export OSNAME OSVERSION_ID OSVERSION OSID OSPRETTY_NAME OSHOME_URL
export TROGLOHUB
export ROOTDIR PATH STAGING PKG_CONFIG_PATH IMAGEDIR DOWNLOADS
export KBUILD_VERBOSE MAKEFLAGS REDIRECT


all: dep staging kernel lib packages ramdisk			## Build all the things

dep:								## Use TroglOS defconfig if user forgets to run menuconfig
	@touch $(BUILDLOG)
	@test -e $(CONFIG) || $(MAKE) defconfig $(REDIRECT)

# Linux Kconfig, menuconfig et al
include kconfig/config.mk

# XXX: Needed only for 'make run', which should be moved to a separate makefile
-include $(CONFIG)
KERNEL_CMDLINE = $(shell echo $(CONFIG_LINUX_CMDLINE) | sed 's/"//g')

# qemu-img create -f qcow hda.img 2G
# +=> -hda hda.img
#			 -drive file=disk.img,if=virtio
#			 -dtb $(IMAGEDIR)/versatile-ab.dtb
#			 -dtb $(IMAGEDIR)/versatile-pb.dtb
run:								## Run Qemu for selected ARCH
	@echo "  QEMU    Ctrl-a x -- exit | Ctrl-a c -- switch console/monitor" | tee -a $(BUILDLOG)
	-@mkdir -p $(PERSISTENT) 2>/dev/null
	-@[ ! -e $(MTD) ] && dd if=/dev/zero of=$(MTD) bs=16384 count=960
	@qemu-system-arm -nographic -m 128M -M versatilepb -usb						\
			 -drive if=scsi,file=$(MTD),format=raw						\
			 -device rtl8139,netdev=nic0							\
			 -netdev bridge,id=nic0,br=virbr0,helper=/usr/lib/qemu/qemu-bridge-helper	\
			 -kernel $(IMAGEDIR)/zImage        						\
			 -initrd $(IMAGEDIR)/initramfs.gz  						\
			 -append "$(KERNEL_CMDLINE) block2mtd.block2mtd=/dev/sda,,Config"

staging:							## Initialize staging area
	@echo "  STAGING Root file system ..." | tee -a $(BUILDLOG)
	@mkdir -p $(IMAGEDIR)
	@mkdir -p $(STAGING)
	@for dir in $(STAGING_DIRS); do   \
		mkdir -p $(STAGING)/$$dir; \
	done
	-@(cd $(STAGING) && ln -sf . usr 2>/dev/null)
	@cp -a $(ROOTDIR)/initramfs/* $(STAGING)/
	@echo "NAME=\"$(OSNAME)\""                     > $(STAGING)/etc/os-release
	@echo "VERSION=\"$(OSVERSION)\""               >>$(STAGING)/etc/os-release
	@echo "ID=\"$(OSID)\""                         >>$(STAGING)/etc/os-release
	@echo "PRETTY_NAME=\"$(OSPRETTY_NAME)\""       >>$(STAGING)/etc/os-release
	@echo "VERSION_ID=\"$(OSVERSION_ID)\""         >>$(STAGING)/etc/os-release
	@echo "HOME_URL=\"$(OSHOME_URL)\""             >>$(STAGING)/etc/os-release
	@echo "SUPPORT_URL=\"$(SUPPORT_URL)\""         >>$(STAGING)/etc/os-release
	@echo "BUG_REPORT_URL=\"$(BUG_REPORT_URL)\""   >>$(STAGING)/etc/os-release
	@echo "$(OSVERSION)"                           > $(STAGING)/etc/version
	@echo "$(OSNAME) $(OSVERSION_ID) \\\\n \\l"    > $(STAGING)/etc/issue
	@echo "$(OSNAME) $(OSVERSION_ID)"              > $(STAGING)/etc/issue.net
	@echo "$(OSRELEASE_ID)"                        > $(STAGING)/etc/hostname
	@sed -i 's/HOSTNAME/$(OSRELEASE_ID)/' $(STAGING)/etc/hosts
	@echo "  INSTALL Toolchain shared libraries ..." | tee -a $(BUILDLOG)
	# XXX: UGLY FIXME!!
	@cp -a /usr/arm-linux-gnueabi/lib/* $(STAGING)/lib/

# Cleanup staging (may need to separate into a staging + romfs dir, like uClinux-dist)
ramdisk:							## Build ramdisk of staging dir
	@echo "  INITRD  $(OSNAME) $(CONFIG_LINUX_VERSION)" | tee -a $(BUILDLOG)
	@rm -rf $(STAGING)/lib/pkgconfig
	@rm -rf $(STAGING)/lib/*.a
	@rm -rf $(STAGING)/share/man
	@touch $(STAGING)/etc/version
	@$(MAKE) -f ramdisk.mk $@ $(REDIRECT)

kernel:								## Build configured Linux kernel
	@$(MAKE) -j5 -C kernel all

kernel_menuconfig:						## Call Linux menuconfig
	@$(MAKE) -C kernel menuconfig

kernel_oldconfig:						## Call Linux oldconfig
	@$(MAKE) -C kernel oldconfig

kernel_defconfig:						## Call Linux defconfig for the selected ARCH
	@$(MAKE) -C kernel defconfig

kernel_saveconfig:						## Save Linux-VER.REV/.config to kernel/config-VER
	@$(MAKE) -C kernel saveconfig

kernel_install:							## Install Linux device tree
	@$(MAKE) -C kernel dtbinst

# Packages may depend on libraries, so we build libs first
packages: lib

packages lib:							## Build packages or libraries
	@$(MAKE) -j5 -C $@ all
	@$(MAKE) -j5 -C $@ install

TARGETS=$(shell find packages -maxdepth 1 -mindepth 1 -type d)
include quick.mk

TARGETS=$(shell find lib -maxdepth 1 -mindepth 1 -type d)
include quick.mk

clean:								## Clean build tree, excluding menuconfig
	@for dir in kernel lib packages; do			\
		echo "  CLEAN   $$dir" | tee -a $(BUILDLOG);	\
		/bin/echo -ne "\033]0;$(PWD) $$dir\007";	\
		$(MAKE) -C $$dir $@ $(REDIRECT);		\
	done

# @$(RM) `find kconfig -name '*~'`
distclean:							## Really clean, as if started from scratch
	@for dir in kconfig kernel lib packages; do		\
		echo "  REMOVE  $$dir" | tee -a $(BUILDLOG);	\
		/bin/echo -ne "\033]0;$(PWD) $$dir\007";	\
		$(MAKE) -C $$dir $@ $(REDIRECT);		\
	done
	-@$(RM) -rf $(STAGING) $(IMAGEDIR) $(CONFIG) $(BUILDLOG)

.PHONY: help
help:
	@grep -hP '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'
	@echo
	@echo 'Briefly, after `git clone`: make all; make run'
	@echo
