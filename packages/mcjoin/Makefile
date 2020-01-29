# Glue for cross compiling multicast test tool mcjoin
PKGVER     := 2.2
PKG        := mcjoin-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = https://github.com/troglobit/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGCFG      = --host=$(CROSS_TARGET) --prefix=
PKGPATCHES :=
PKGINSTALL := install-exec

include $(ROOTDIR)/system/pkg.mk
