# Glue for cross compiling libite (LITE)
PKGVER     := 2.0.2
PKG        := libite-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = https://github.com/troglobit/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGCFG      = --host=$(CROSS_TARGET) --prefix=

include $(ROOTDIR)/system/pkg.mk
