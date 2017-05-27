# Glue for cross compiling multicast test tool mcjoin
include $(ROOTDIR)/core.mk

PKGVER     := 2.2
PKG        := mcjoin-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGCFG     := --host=$(CROSS_TARGET) --prefix=
PKGPATCHES :=
PKGINSTALL := install-exec

include $(ROOTDIR)/pkg.mk
