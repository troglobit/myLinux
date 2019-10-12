# Glue for cross compiling libnet
include $(ROOTDIR)/core.mk

PKGVER     := 1.2-rc4
PKG        := libnet-$(PKGVER)
PKGSUFFIX  := tar.gz
PKGURL      = https://github.com/libnet/libnet/releases/download/v$(PKGVER)/$(PKG).$(PKGSUFFIX)
PKGCFG     := --host=$(CROSS_TARGET) --prefix=

include $(ROOTDIR)/pkg.mk
