# Glue for cross compiling libnet
include $(ROOTDIR)/core.mk

PKGVER     := 1.1.6
PKG        := libnet-$(PKGVER)
PKGSUFFIX  := tar.gz
PKGURL      = http://deb.debian.org/debian/pool/main/libn/libnet/libnet_1.1.6+dfsg.orig.tar.gz
PKGCFG     := --host=$(CROSS_TARGET) --prefix=

include $(ROOTDIR)/pkg.mk
