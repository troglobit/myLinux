# Glue for creating a bootable Macchiatobin SD card
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
	@cp -v $(IMAGEDIR)/Image mnt/
	@cp -v $(IMAGEDIR)/marvell/*.dtb mnt/
	@cp -v $(IMAGEDIR)/initramfs.* mnt/
	@umount mnt
	@rmdir mnt

