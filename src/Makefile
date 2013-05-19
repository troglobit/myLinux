CC=$(CROSS_COMPILE)gcc
CFLAGS=-g -Wall -W -Werror -std=gnu99 $(DEFINES)
BUSYBOX_VERSION=1.21.0
MINISNMP_VERSION=1.2b
DROPBEAR_VERSION=2013.58
FSNAME=initramfs-bin
RELEASEDIR=../../images/
BOARD=EOMA68

PROGRAMS= busybox
all: $(PROGRAMS)

clean: 
	rm -rf busybox-$(BUSYBOX_VERSION)
	rm -rf dropbear-$(DROPBEAR_VERSION)
	rm -rf mini_snmpd
	rm -f $(PROGRAMS) *.o

.PHONY: all clean

.PHONY: busybox 

busybox: busybox-$(BUSYBOX_VERSION)/busybox

busybox-$(BUSYBOX_VERSION)/busybox:	busybox-$(BUSYBOX_VERSION)/.config 
	$(MAKE) -C busybox-$(BUSYBOX_VERSION)

busybox-$(BUSYBOX_VERSION)/.config: busybox.config busybox-$(BUSYBOX_VERSION)/.unpacked
	cp busybox.config busybox-$(BUSYBOX_VERSION)/.config

busybox-$(BUSYBOX_VERSION)/.unpacked: busybox-$(BUSYBOX_VERSION).tar.bz2 patches/busybox/*
	tar jxf busybox-$(BUSYBOX_VERSION).tar.bz2
	for patch in patches/busybox/*.patch; do \
		echo "$$patch"; \
		patch -p1 -d busybox-$(BUSYBOX_VERSION) < $$patch; \
	done
	touch busybox-$(BUSYBOX_VERSION)/.unpacked

	if [ ! -d busybox-$(BUSYBOX_VERSION) ]; then tar jxf busybox-$(BUSYBOX_VERSION).tar.bz2; fi

busybox-$(BUSYBOX_VERSION).tar.bz2:
	wget http://www.busybox.net/downloads/busybox-$(BUSYBOX_VERSION).tar.bz2

busybox_install: busybox-$(BUSYBOX_VERSION)/busybox
	$(MAKE) -C busybox-$(BUSYBOX_VERSION)  CONFIG_PREFIX=$(shell pwd)/../$(FSNAME)/ install

.PHONY: mini_snmpd

mini_snmpd: mini_snmpd/mini_snmpd

mini_snmpd/mini_snmpd: mini_snmpd/.unpacked
	$(MAKE) -C mini_snmpd OFLAGS="-O2 -D__IPV4__ -static" CC=$(CC)

mini_snmpd/.unpacked: mini_snmpd-$(MINISNMP_VERSION).tar.gz patches/mini_snmpd/*
	tar zxf mini_snmpd-$(MINISNMP_VERSION).tar.gz
	for patch in patches/mini_snmpd/*.patch; do \
		echo "$$patch"; \
		patch -d mini_snmpd < $$patch; \
	done
	touch mini_snmpd/.unpacked

mini_snmpd-$(MINISNMP_VERSION).tar.gz:
	wget http://members.aon.at/linuxfreak/linux/mini_snmpd-$(MINISNMP_VERSION).tar.gz

dropbear: dropbear-$(DROPBEAR_VERSION)/.unpacked

dropbear-$(DROPBEAR_VERSION)/.unpacked: dropbear-$(DROPBEAR_VERSION).tar.bz2
	tar jxf dropbear-$(DROPBEAR_VERSION).tar.bz2
	touch dropbear-$(DROPBEAR_VERSION)/.unpacked

dropbear-$(DROPBEAR_VERSION).tar.bz2:
	wget https://matt.ucc.asn.au/dropbear/releases/dropbear-$(DROPBEAR_VERSION).tar.bz2:
	
install: $(PROGRAMS) busybox_install ../$(FSNAME)
#	$(CROSS_COMPILE)strip -o ../$(FSNAME)/sbin/mini_snmpd mini_snmpd/mini_snmpd

installfs:
	$(MAKE) FSNAME=installfs do_installfs

# Never call this directly. Use installfs above
do_installfs: $(PROGRAMS) busybox_install ../$(FSNAME)
	mkdir -p ../$(FSNAME)/install
	tar -C ../$(FSNAME)/install -zxvf $(RELEASEDIR)/$(BOARD).img

INITRAMFS_DIRS=../$(FSNAME)/mnt ../$(FSNAME)/proc ../$(FSNAME)/sys ../$(FSNAME)/bin ../$(FSNAME)/sbin ../$(FSNAME)/tmp

../$(FSNAME): $(INITRAMFS_DIRS) ../$(FSNAME)/usr

$(INITRAMFS_DIRS):
	mkdir $@

../$(FSNAME)/usr:
	ln -s . $@

cleaninstallfs:
	$(MAKE) FSNAME=installfs cleaninitramfs

cleaninitramfs:	
	find ../$(FSNAME) -type l -print | xargs rm -f
	rm -f ../$(FSNAME)/bin/busybox
	rm -f ../$(FSNAME)/install/mtd*
	rm -f ../$(FSNAME)/install/run*
	rmdir ../$(FSNAME)/* 2>/dev/null || true


realclean: cleaninitramfs cleaninstallfs clean
	rm -f *.tar.*
