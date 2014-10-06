CROSS_COMPILE=/home/henrik/toolchains/arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
ARCH=arm
export CROSS_COMPILE ARCH
ifdef KERNEL_MODULES
KERNELRELEASE = $(shell $(MAKE) -s -C $(KERNEL_MODULES)/build CROSS_COMPILE=$(CROSS_COMPILE) ARCH=$(ARCH) kernelrelease)
else
KERNELRELEASE = $(error please define KERNEL_MODULES to where the kernel modules are installed)
endif

all: initramfs-$(KERNELRELEASE).img initramfs-$(KERNELRELEASE).uImage

mrproper: src/mrproper clean

.PHONY: modules

modules: 
	rm -rf modules
	mkdir -p modules/lib/modules/$(KERNELRELEASE)
	sh -ec "cd $(KERNEL_MODULES);find * -depth -print0 | cpio -0pdm $(PWD)/modules/lib/modules/$(KERNELRELEASE)"
	find modules/lib/modules/$(KERNELRELEASE) -name '*.ko' | xargs $(CROSS_COMPILE)strip -g

initramfs-$(KERNELRELEASE).gz: initramfs-$(KERNELRELEASE).cpio
	gzip -9 <initramfs-$(KERNELRELEASE).cpio >initramfs-$(KERNELRELEASE).gz

initramfs-$(KERNELRELEASE).lzo: initramfs-$(KERNELRELEASE).cpio
	lzop -9 -o initramfs-$(KERNELRELEASE).lzo initramfs-$(KERNELRELEASE).cpio

initramfs-$(KERNELRELEASE).img: initramfs-$(KERNELRELEASE).gz
	mkimage -T ramdisk -A arm -C none -d $< initramfs-$(KERNELRELEASE).img

initramfs-$(KERNELRELEASE).cpio: modules src/install
	( cat initramfs.devnodes ; sh $(KERNEL_MODULES)/build/source/scripts/gen_initramfs_list.sh -u squash -g squash initramfs-bin/ initramfs/ modules/ ) | \
	$(KERNEL_MODULES)/build/usr/gen_init_cpio - >initramfs-$(KERNELRELEASE).cpio

initramfs-$(KERNELRELEASE).uImage: modules
	cp -p $(KERNEL_MODULES)/build/arch/arm/boot/uImage $@

clean:
	rm -rf modules initramfs-bin initramfs*.cpio initramfs*.gz initramfs*.lzo initramfs*.img

clean: src/clean

src/%:
	$(MAKE) -C src $*

