.PHONY: ramdisk

include $(ROOTDIR)/system/core.mk

# images/initramfs.img images/initramfs.uImage
RAMDISK := images/initramfs.gz
RAMIMG  := images/initramfs.img

ramdisk: $(RAMDISK) $(RAMIMG)
	@ln -f $(RAMDISK) images/initramfs.gz
	@ln -f $(RAMIMG)  images/initramfs.img

# http://free-electrons.com/blog/uncompressed-linux-kernel-on-arm/
images/initramfs.uImage:
	@cp -p $(KERNEL_MODULES)/build/arch/arm/boot/uImage $@

images/initramfs.gz: images/initramfs.cpio
	@gzip -9 < $< >$@

images/initramfs.lzo: images/initramfs.cpio
	@lzop -9 -o $@ $<

images/initramfs.img: images/initramfs.cpio
	@mkimage -T ramdisk -A $(ARCH) -C none -n uInitrd -d $< $@

images/initramfs.cpio: $(ROMFS)/etc/version
	@(cat initramfs.devnodes; \
	  sh $(KERNEL_MODULES)/build/usr/gen_initramfs_list.sh -u squash -g squash $(ROMFS)) \
	   | $(KERNEL_MODULES)/build/usr/gen_init_cpio - >$@
