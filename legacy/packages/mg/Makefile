# Glue for cross compiling Mg
PKGVER     := 3.0
PKG        := mg-$(PKGVER)
PKGSUFFIX  := tar.gz
PKGURL      = https://github.com/troglobit/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGPATCHES :=
PKGCFG      = --host=$(CROSS_TARGET) --prefix=
PKGCFG     += --disable-all --enable-size-optimizations
PKGINSTALL := install-exec

include $(ROOTDIR)/system/pkg.mk

install::
	@ln -sf mg $(STAGING)/bin/emacs
