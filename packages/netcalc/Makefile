# Glue for cross compiling netcalc

PKGVER     := 2.0
PKG        := netcalc-$(PKGVER)
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGCFG     := --host=$(CROSS_TARGET) --prefix=
PKGINSTALL := install-exec

include $(ROOTDIR)/pkg.mk
