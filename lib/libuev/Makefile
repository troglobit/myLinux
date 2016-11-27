# Glue for cross compiling libuEv

PKGVER     := 1.5.1
PKG        := libuev-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGCFG     := --host=$(CROSS_TARGET) --prefix=

include $(ROOTDIR)/pkg.mk
