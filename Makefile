# Top level Makefile
#
# Copyright (c) 2014-2017  Joachim Nilsson <troglobit@gmail.com>
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
.PHONY: help run world staging boot kernel romfs ramdisk clean distclean image

OSNAME            := TroglOS Linux
OSRELEASE_ID      := chaos
OSRELEASE          = Chaos Devel `date --iso-8601`
OSVERSION_ID      := 1.0-rc1
OSVERSION         := $(OSVERSION_ID), $(OSRELEASE)
OSID              := "troglos"
OSPRETTY_NAME     := $(OSNAME) $(OSVERSION_ID)
OSHOME_URL        := https://troglobit.com
TROGLOHUB         := https://github.com/troglobit
SUPPORT_URL       := $(TROGLOHUB)/troglos
BUG_REPORT_URL    := $(TROGLOHUB)/troglos/issues

ROOTDIR           := $(shell pwd)
PATH              := $(ROOTDIR)/bin:$(PATH)
srctree           := $(ROOTDIR)

# usr/lib usr/share usr/bin usr/sbin
STAGING_DIRS       = mnt proc sys lib share bin sbin tmp var home host

# Include .config variables, unless calling Kconfig
noconfig_targets  := menuconfig nconfig gconfig xconfig config oldconfig	\
		     defconfig %_defconfig allyesconfig allnoconfig distclean

ifdef V
  ifeq ("$(origin V)", "command line")
    KBUILD_VERBOSE = $(V)
  endif
endif
ifndef KBUILD_VERBOSE
  KBUILD_VERBOSE   = 0
endif
KBUILD_DEFCONFIG   = defconfig
ifeq ($(KBUILD_VERBOSE),1)
MAKEFLAGS          =
else
MAKEFLAGS          = --silent --no-print-directory
endif

export OSNAME OSRELEASE_ID OSRELEASE OSVERSION_ID OSVERSION
export OSID OSPRETTY_NAME OSHOME_URL
export PATH ROOTDIR srctree STAGING_DIRS
export TROGLOHUB SUPPORT_URL BUG_REPORT_URL
export KBUILD_VERBOSE MAKEFLAGS

ifeq ($(filter $(noconfig_targets),$(MAKECMDGOALS)),)
ifeq (.config, $(wildcard .config))
include $(ROOTDIR)/.config
all: dep staging kernel world image				## Build all the things
else
all: error
endif
endif

dep:
	+@make -C arch $@

# Linux Kconfig, menuconfig et al
include kconfig/config.mk

run:								## Run Qemu for selected target platform
	@$(MAKE) -C arch $@

staging:							## Initialize staging area
	+@$(MAKE) -C arch $@

romfs:								## Create stripped down romfs/ from staging/
	+@$(MAKE) -C arch $@

sdcard:								## Create Raspberry Pi SD card
	@$(MAKE) -C arch $@

ramdisk:							## Build ramdisk of staging dir
	@echo "  INITRD  $(OSNAME) $(OSVERSION_ID)"
	@touch romfs/etc/version
	@$(MAKE) -f ramdisk.mk $@

image: world							## Build image, with dependency checking
	+@$(MAKE) -C arch $@

kernel:								## Build configured Linux kernel
	+@$(MAKE) -C kernel all
	+@$(MAKE) kernel_install

kernel_menuconfig:						## Linux menuconfig
	@$(MAKE) -C kernel menuconfig

kernel_oldconfig:						## Linux oldconfig
	@$(MAKE) -C kernel oldconfig

kernel_defconfig:						## Linux defconfig for the selected target platform
	@$(MAKE) -C kernel defconfig

kernel_saveconfig:						## Save Linux-VER.REV/.config to kernel/config-VER
	@$(MAKE) -C kernel saveconfig

kernel_install:							## Install Linux device tree
	+@$(MAKE) -C kernel dtbinst

world:								## Build everything, in order
	+@for dir in lib boot packages user; do			\
		$(MAKE) -C $$dir all;				\
		$(MAKE) -C $$dir install;			\
	done

TARGETS=$(shell find lib -maxdepth 1 -mindepth 1 -type d)
include quick.mk

TARGETS=$(shell find packages -maxdepth 1 -mindepth 1 -type d)
include quick.mk

TARGETS=$(shell find user -maxdepth 1 -mindepth 1 -type d)
include quick.mk

clean:								## Clean build tree, excluding menuconfig
	-+@for dir in user packages lib kernel boot; do		\
		echo "  CLEAN   $$dir";				\
		$(MAKE) -C $$dir $@;				\
	done

distclean:							## Really clean, as if started from scratch
	-+@for dir in user packages lib kernel boot kconfig; do	\
		echo "  PURGE   $$dir";				\
		$(MAKE) -C $$dir $@;				\
	done
	-+@for file in .config staging romfs images; do 	\
		echo "  PURGE   $$file";	   		\
		$(RM) -rf $$file;				\
	done

error:
	@echo "  FAIL    No .config found, see the README for help on download and set up."
	@exit 1

help:
	@grep -hP '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST)	\
	| sort | awk 'BEGIN {FS = ":.*?## "}; 			\
			    {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'
	@echo
	@echo 'Briefly, after `git clone`: make all; make run'
	@echo

# Disable parallel build in this Makefile only, to ensure execution
# of 'image' *after* 'world' --J
.NOTPARALLEL:
