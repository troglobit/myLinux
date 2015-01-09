.PHONY: ramdisk

KERNEL_MODULES  = $(STAGING)/lib/modules/$(KERNEL_VERSION)
KERNELRELEASE   = $(shell test -d $(KERNEL_MODULES)/build && $(MAKE) -s -C $(KERNEL_MODULES)/build CROSS_COMPILE=$(CROSS_COMPILE) ARCH=$(ARCH) kernelrelease)

# images/initramfs-$(KERNELRELEASE).img images/initramfs-$(KERNELRELEASE).uImage
RAMDISK         = images/initramfs-$(KERNELRELEASE).gz

ramdisk: $(RAMDISK)
	@ln -sf `basename $(RAMDISK)` images/initramfs.gz

# http://free-electrons.com/blog/uncompressed-linux-kernel-on-arm/
images/initramfs-$(KERNELRELEASE).uImage:
	@cp -p $(KERNEL_MODULES)/build/arch/arm/boot/uImage $@

images/initramfs-$(KERNELRELEASE).gz: images/initramfs-$(KERNELRELEASE).cpio
	@gzip -9 < $< >$@

images/initramfs-$(KERNELRELEASE).lzo: images/initramfs-$(KERNELRELEASE).cpio
	@lzop -9 -o $@ $<

images/initramfs-$(KERNELRELEASE).img: images/initramfs-$(KERNELRELEASE).gz
	@mkimage -T ramdisk -A arm -C none -d $< $@

images/initramfs-$(KERNELRELEASE).cpio: $(STAGING)/etc/version
	@(cat initramfs.devnodes; \
		sh $(KERNEL_MODULES)/build/scripts/gen_initramfs_list.sh -u squash -g squash $(STAGING)) \
		 | $(KERNEL_MODULES)/build/usr/gen_init_cpio - >$@
