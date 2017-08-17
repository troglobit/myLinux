.PHONY: sdcard
sdcard:
	if [ "`id -nu`" != "root" ]; then				\
		echo "$(MAKE) $@ requires root perimissions, try sudo";	\
		exit 1;							\
	fi
	@echo "  SDCARD  $(CONFIG_MACH)" | tee -a $(BUILDLOG)
	@mksdcard /dev/mmcblk0
	-@mkdir mnt
	-@mount /dev/mmcblk0p1 mnt
	@mkimage -C none -A arm -T script -d boot.cmd mnt/boot.scr
	@cp -v $(IMAGEDIR)/boot/* mnt/
	@cp -v $(IMAGEDIR)/zImage mnt/
	@cp -v $(IMAGEDIR)/*.dtb  mnt/
	@cp -v $(IMAGEDIR)/u-boot.bin mnt/kernel.img
	@cp -v $(IMAGEDIR)/initramfs.img mnt/
	@umount mnt
	@rmdir mnt

