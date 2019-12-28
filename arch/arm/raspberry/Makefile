# Glue for creating a bootable Raspberry Pi SD card
.PHONY: sdcard

include $(ROOTDIR)/core.mk

sdcard:
	if [ "`id -nu`" != "root" ]; then				\
		echo "$(MAKE) $@ requires root perimissions, try sudo";	\
		exit 1;							\
	fi
	@echo "  SDCARD  $(CONFIG_MACH)"
	@mksdcard $(CONFIG_SDCARD)
	-@mkdir mnt
	-@mount $(CONFIG_SDCARD)p1 mnt
	@mkimage -C none -A $(CONFIG_ARCH) -T script -d boot.cmd mnt/boot.scr
	@cp -v $(IMAGEDIR)/boot/* mnt/
	@cp -v $(IMAGEDIR)/zImage mnt/
	@cp -v $(IMAGEDIR)/*.dtb  mnt/
	@cp -v $(IMAGEDIR)/u-boot.bin mnt/kernel.img
	@cp -v $(IMAGEDIR)/initramfs.img mnt/
	@umount mnt
	@rmdir mnt

