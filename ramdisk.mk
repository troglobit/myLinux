.PHONY: ramdisk

include core.mk

# images/initramfs-$(KERNELRELEASE).img images/initramfs-$(KERNELRELEASE).uImage
RAMDISK := images/initramfs-$(KERNELRELEASE).gz images/initramfs-$(KERNELRELEASE).img

ramdisk: $(RAMDISK)
	@ln -sf `basename $(RAMDISK)` images/initramfs.gz

# http://free-electrons.com/blog/uncompressed-linux-kernel-on-arm/
images/initramfs-$(KERNELRELEASE).uImage:
	@cp -p $(KERNEL_MODULES)/build/arch/arm/boot/uImage $@

images/initramfs-$(KERNELRELEASE).gz: images/initramfs-$(KERNELRELEASE).cpio
	@gzip -9 < $< >$@

images/initramfs-$(KERNELRELEASE).lzo: images/initramfs-$(KERNELRELEASE).cpio
	@lzop -9 -o $@ $<

images/initramfs-$(KERNELRELEASE).img: images/initramfs-$(KERNELRELEASE).cpio
	@mkimage -T ramdisk -A arm -C none -n uInitrd -d $< $@

images/initramfs-$(KERNELRELEASE).cpio: $(ROMFS)/etc/version
	@(cat initramfs.devnodes; \
		sh $(KERNEL_MODULES)/build/scripts/gen_initramfs_list.sh -u squash -g squash $(ROMFS)) \
		 | $(KERNEL_MODULES)/build/usr/gen_init_cpio - >$@
