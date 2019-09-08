# Glue for cross compiling libmnl, a minimalistic user-space Netlink library

PKGVER     := 1.0.4
PKG        := libmnl-$(PKGVER)
PKGSUFFIX  := tar.bz2
PKGURL      = http://ftp.netfilter.org/pub/libmnl/v$(PKGVER)/$(PKGTAR)
PKGCFG     := --host=$(CROSS_TARGET) --prefix=

include $(ROOTDIR)/pkg.mk
