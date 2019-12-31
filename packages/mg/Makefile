# Glue for cross compiling Mg
PKGVER     := 3.0
PKG        := mg-$(PKGVER)
PKGSUFFIX  := tar.gz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGPATCHES :=
PKGCFG      = --host=$(CROSS_TARGET) --prefix=
PKGCFG     += --disable-all --enable-size-optimizations
PKGINSTALL := install-exec

install::
	@ln -sf mg $(STAGING)/bin/emacs

include $(ROOTDIR)/pkg.mk
