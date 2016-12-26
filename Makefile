# Top level Makefile
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
.SUFFIX:
.PHONY: run staging kernel lib packages romfs ramdisk clean distclean

OSNAME            := TroglOS Linux
OSRELEASE_ID      := chaos
OSRELEASE          = Chaos Devel `date --iso-8601`
OSVERSION_ID      := 1.0-rc1
OSVERSION         := $(OSVERSION_ID), $(OSRELEASE)
OSID              := "troglos"
OSPRETTY_NAME     := $(OSNAME) $(OSVERSION_ID)
OSHOME_URL        := http://troglobit.com
TROGLOHUB         := https://github.com/troglobit
SUPPORT_URL       := $(TROGLOHUB)/troglos
BUG_REPORT_URL    := $(TROGLOHUB)/troglos/issues

ROOTDIR           := $(shell pwd)
PATH              := $(ROOTDIR)/bin:$(PATH)
BUILDLOG          := $(ROOTDIR)/build.log
srctree           := $(ROOTDIR)

# usr/lib usr/share usr/bin usr/sbin 
STAGING_DIRS       = mnt proc sys lib share bin sbin tmp var home

ifdef V
  ifeq ("$(origin V)", "command line")
    KBUILD_VERBOSE = $(V)
  endif
endif
ifndef KBUILD_VERBOSE
  KBUILD_VERBOSE   = 0
endif
ifeq ($(KBUILD_VERBOSE),1)
MAKEFLAGS          =
REDIRECT           = 2>&1 | teepee $(BUILDLOG)
else
MAKEFLAGS          = --silent --no-print-directory
REDIRECT           = >> $(BUILDLOG) 2>&1
endif

export OSNAME OSRELEASE_ID OSRELEASE OSVERSION_ID OSVERSION
export OSID OSPRETTY_NAME OSHOME_URL
export PATH ROOTDIR BUILDLOG srctree
export TROGLOHUB SUPPORT_URL BUG_REPORT_URL
export KBUILD_VERBOSE MAKEFLAGS REDIRECT

#include core.mk

all: dep staging kernel lib packages user romfs ramdisk		## Build all the things

dep:								## Use TroglOS defconfig if user forgets to run menuconfig
	@touch $(BUILDLOG)
	@test -e .config || $(MAKE) ARCH=arm versatile_defconfig

# Linux Kconfig, menuconfig et al
include kconfig/config.mk

run:								## Run Qemu for selected target platform
	@$(MAKE) -C arch $@

staging:							## Initialize staging area
	@$(MAKE) -C arch $@

romfs:								## Create stripped down romfs/ from staging/
	@$(MAKE) -C arch $@

ramdisk:							## Build ramdisk of staging dir
	@echo "  INITRD  $(OSNAME) $(OSVERSION_ID)" | tee -a $(BUILDLOG)
	@touch romfs/etc/version
	@$(MAKE) -f ramdisk.mk $@ $(REDIRECT)

kernel:								## Build configured Linux kernel
	@$(MAKE) -j5 -C kernel all
	@$(MAKE) kernel_install

kernel_menuconfig:						## Call Linux menuconfig
	@$(MAKE) -C kernel menuconfig

kernel_oldconfig:						## Call Linux oldconfig
	@$(MAKE) -C kernel oldconfig

kernel_defconfig:						## Call Linux defconfig for the selected target platform
	@$(MAKE) -C kernel defconfig

kernel_saveconfig:						## Save Linux-VER.REV/.config to kernel/config-VER
	@$(MAKE) -C kernel saveconfig

kernel_install:							## Install Linux device tree
	@$(MAKE) -C kernel dtbinst

# Packages may depend on libraries, so we build libs first
packages: lib

# We don't know anything about user programs, we build them last
user: packages lib

user packages lib:						## Build packages or libraries
	@$(MAKE) -j5 -C $@ all
	@$(MAKE) -j5 -C $@ install

TARGETS=$(shell find lib -maxdepth 1 -mindepth 1 -type d)
include quick.mk

TARGETS=$(shell find packages -maxdepth 1 -mindepth 1 -type d)
include quick.mk

TARGETS=$(shell find user -maxdepth 1 -mindepth 1 -type d)
include quick.mk

clean:								## Clean build tree, excluding menuconfig
	@for dir in kernel lib packages user; do		\
		echo "  CLEAN   $$dir" | tee -a $(BUILDLOG);	\
		/bin/echo -ne "\033]0;$(PWD) $$dir\007";	\
		$(MAKE) -C $$dir $@ $(REDIRECT);		\
	done

# @$(RM) `find kconfig -name '*~'`
distclean:							## Really clean, as if started from scratch
	@for dir in kconfig kernel lib packages user; do	\
		echo "  REMOVE  $$dir" | tee -a $(BUILDLOG);	\
		/bin/echo -ne "\033]0;$(PWD) $$dir\007";	\
		$(MAKE) -C $$dir $@ $(REDIRECT);		\
	done
	-@$(RM) -rf .config $(STAGING) $(ROMFS) $(IMAGEDIR) $(BUILDLOG)

.PHONY: help
help:
	@grep -hP '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'
	@echo
	@echo 'Briefly, after `git clone`: make all; make run'
	@echo
