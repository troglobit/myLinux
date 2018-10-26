# Glue for cross compiling netcalc
include $(ROOTDIR)/core.mk

PKGVER     := 2.1.4
PKG        := netcalc-$(PKGVER)
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGPATCHES :=
PKGCFG     := --host=$(CROSS_TARGET) --prefix= --disable-ipcalc-symlink
PKGINSTALL := install-exec

include $(ROOTDIR)/pkg.mk
