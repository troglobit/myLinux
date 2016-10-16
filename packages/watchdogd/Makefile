# Glue for cross compiling watchdogd

PKGVER     := 3.0-beta1
PKG        := watchdogd-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/$(PKGVER)/$(PKGTAR)
PKGCFG     := --host=$(CROSS_TARGET) --prefix=
PKGCFG     += --disable-test-mode --enable-examples
PKGINSTALL := install-exec

include $(ROOTDIR)/pkg.mk
