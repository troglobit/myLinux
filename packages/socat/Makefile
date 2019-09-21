# Glue for cross compiling socat
include $(ROOTDIR)/core.mk

PKGVER     := 1.7.3.3
PKG        := socat-$(PKGVER)
PKGSUFFIX  := tar.bz2
PKGURL      = http://www.dest-unreach.org/socat/download/$(PKGTAR)
PKGCFG     := --host=$(CROSS_TARGET) --prefix= 

include $(ROOTDIR)/pkg.mk
