# Glue for cross compiling mrouted
include $(ROOTDIR)/core.mk

PKGVER     := 3.9.8
PKG        := mrouted-$(PKGVER)
PKGSUFFIX  := tar.bz2
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/$(PKGVER)/$(PKGTAR)
PKGPATCHES :=
PKGCFG     := --enable-rsrr

include $(ROOTDIR)/pkg.mk
