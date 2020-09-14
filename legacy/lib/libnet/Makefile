# Glue for cross compiling libnet
PKGVER     := 1.2
PKG        := libnet-$(PKGVER)
PKGSUFFIX  := tar.gz
PKGURL      = https://github.com/libnet/libnet/releases/download/v$(PKGVER)/$(PKG).$(PKGSUFFIX)
PKGCFG      = --host=$(CROSS_TARGET) --prefix=

include $(ROOTDIR)/system/pkg.mk
