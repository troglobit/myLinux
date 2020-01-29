# Glue for cross compiling libuEv
PKGVER     := 2.2.0
PKG        := libuev-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = https://github.com/troglobit/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGCFG      = --host=$(CROSS_TARGET) --prefix=

include $(ROOTDIR)/system/pkg.mk
