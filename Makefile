############################################################################
# Top level Makefile
#
# Copyright (c) 2013-2014  Henrik Nordström <henrik@henriknordstrom.net>
# Copyright (c) 2014-2015  Joachim Nilsson <troglobit@gmail.com
#
############################################################################
.PHONY: staging modules kernel mrproper clean run

ARCH            = arm
CROSS_COMPILE  ?= arm-linux-gnueabi-
MAKEFLAGS       = --silent --no-print-directory

ROOTDIR        := $(shell pwd)
STAGING         = $(ROOTDIR)/staging
STAGING_DIRS    = mnt proc sys bin sbin tmp
IMAGEDIR        = $(ROOTDIR)/images
DOWNLOADS      ?= $(shell xdg-user-dir DOWNLOAD 2>/dev/null || echo "$(ROOTDIR)/downloads")

KERNEL_VERSION  = 3.14.4
KERNEL_MODULES  = $(STAGING)/lib/modules/$(KERNEL_VERSION)
KERNELRELEASE   = $(shell $(MAKE) -s -C $(KERNEL_MODULES)/build CROSS_COMPILE=$(CROSS_COMPILE) ARCH=$(ARCH) kernelrelease)

export ARCH CROSS_COMPILE
export KERNEL_VERSION
export ROOTDIR STAGING IMAGEDIR DOWNLOADS

all: staging kernel headers packages modules initramfs-$(KERNELRELEASE).img initramfs-$(KERNELRELEASE).uImage

run:
	@echo "  QEMU    $(ARCH) ..."
	@qemu-system-arm -m 128M -M versatilepb -kernel $(IMAGEDIR)/zImage -initrd initramfs.gz -append "root=/dev/ram"

staging:
	-@mkdir $(IMAGEDIR) 2>/dev/null
	-@mkdir $(STAGING)  2>/dev/null
	@for dir in $(STAGING_DIRS); do \
		mkdir $(STAGING)/$$dir; \
	done
	@(cd $(STAGING) && ln -s . usr)
	@cp -a $(ROOTDIR)/initramfs/* $(STAGING)/

kernel:
	@$(MAKE) -C $@ all

modules: 
	rm -rf modules
	mkdir -p modules/lib/modules/$(KERNELRELEASE)
	sh -ec "cd $(KERNEL_MODULES);find * -depth -print0 | cpio -0pdm $(PWD)/modules/lib/modules/$(KERNELRELEASE)"
	find modules/lib/modules/$(KERNELRELEASE) -name '*.ko' | xargs $(CROSS_COMPILE)strip -g

busybox_menuconfig:
	@$(MAKE) -C packages $@

packages:
	@$(MAKE) -C packages all

initramfs-$(KERNELRELEASE).gz: initramfs-$(KERNELRELEASE).cpio
	@gzip -9 <initramfs-$(KERNELRELEASE).cpio >initramfs-$(KERNELRELEASE).gz

initramfs-$(KERNELRELEASE).lzo: initramfs-$(KERNELRELEASE).cpio
	@lzop -9 -o initramfs-$(KERNELRELEASE).lzo initramfs-$(KERNELRELEASE).cpio

initramfs-$(KERNELRELEASE).img: initramfs-$(KERNELRELEASE).gz
	@mkimage -T ramdisk -A arm -C none -d $< initramfs-$(KERNELRELEASE).img

initramfs-$(KERNELRELEASE).cpio: modules
	@(cat initramfs.devnodes; \
		sh $(KERNEL_MODULES)/build/scripts/gen_initramfs_list.sh -u squash -g squash $(STAGING)) \
		 | $(KERNEL_MODULES)/build/usr/gen_init_cpio - >initramfs-$(KERNELRELEASE).cpio

initramfs-$(KERNELRELEASE).uImage: modules
	cp -p $(KERNEL_MODULES)/build/arch/arm/boot/uImage $@

clean:
	@for dir in kernel packages; do	\
		$(MAKE) -C $$dir $@;	\
	done
	@rm -rf initramfs-bin initramfs.cpio initramfs.gz initramfs.lzo initramfs.img $(STAGING)

distclean:
	@for dir in kernel packages; do	\
		$(MAKE) -C $$dir $@;	\
	done

packages/%:
	@$(MAKE) -C packages $*

