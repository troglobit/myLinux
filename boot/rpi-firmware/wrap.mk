# Simple Makefile wrapper for raspberry/firmware
FIRMWARE := bootcode.bin fixup.dat start.elf

all:
	@true

install:
	@mkdir -p $(IMAGEDIR)/boot
	@for file in $(FIRMWARE); do 			\
		cp -fv boot/$$file $$IMAGEDIR/boot/;	\
	done
