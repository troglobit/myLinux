# Glue for cross compiling libmnl, a minimalistic user-space Netlink library
include $(ROOTDIR)/core.mk

PKGVER     := 1.0.4
PKG        := libmnl-$(PKGVER)
PKGSUFFIX  := tar.bz2
PKGURL      = http://ftp.netfilter.org/pub/libmnl/$(PKGTAR)
PKGCFG     := --host=$(CROSS_TARGET) --prefix=$(STAGING)

include $(ROOTDIR)/pkg.mk
