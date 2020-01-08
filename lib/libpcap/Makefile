# Glue for cross compiling libpcap
PKGVER     := 1.6.1
PKG        := libpcap-$(PKGVER)
PKGSUFFIX  := tar.gz
PKGURL      = http://www.tcpdump.org/release/$(PKGTAR)
PKGCFG      = --host=$(CROSS_TARGET) --prefix=
PKGCFG     += --with-pcap=linux --without-snf --without-septel --without-dag
PKGCFG     += --disable-protochain --disable-largefile --enable-yydebug
PKGCFG     += --disable-bluetooth --disable-dbus
PKGCFG     += --disable-canusb --disable-can

include $(ROOTDIR)/system/pkg.mk

