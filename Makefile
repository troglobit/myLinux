CROSS_COMPILE=/home/henrik/toolchains/arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
ARCH=arm
export CROSS_COMPILE ARCH

all: initramfs.img

mrproper: src/mrproper clean

.PHONY: modules

KERNELRELEASE = $(shell $(MAKE) -s -C $(KERNEL_MODULES)/build CROSS_COMPILE=$(CROSS_COMPILE) ARCH=$(ARCH) kernelrelease)
modules: 
ifndef KERNEL_MODULES
	$(error please define KERNEL_MODULES to where the kernel modules are installed)
endif
	rm -rf modules
	mkdir -p modules/lib/modules/$(KERNELRELEASE)
	sh -ec "cd $(KERNEL_MODULES);find * -depth -print0 | cpio -0pdm $(PWD)/modules/lib/modules/$(KERNELRELEASE)"
	find modules/lib/modules/$(KERNELRELEASE) -name '*.ko' | xargs $(CROSS_COMPILE)strip -g

initramfs.gz: initramfs.cpio
	gzip -9 <initramfs.cpio >initramfs.gz

initramfs.lzo: initramfs.cpio
	lzop -9 -o initramfs.lzo initramfs.cpio

initramfs.img: initramfs.gz
	mkimage -T ramdisk -A arm -C none -d $< initramfs.img

initramfs.cpio: modules src/install
	( cat initramfs.devnodes ; sh $(KERNEL_MODULES)/build/source/scripts/gen_initramfs_list.sh -u squash -g squash initramfs-bin/ initramfs/ modules/ ) | \
	$(KERNEL_MODULES)/build/usr/gen_init_cpio - >initramfs.cpio

clean:
	rm -rf modules initramfs-bin initramfs.cpio initramfs.gz initramfs.lzo initramfs.img

clean: src/clean

src/%:
	$(MAKE) -C src $*

