# Glue for cross compiling netcalc
PKGVER     := 2.1.4
PKG        := netcalc-$(PKGVER)
PKGURL      = https://github.com/troglobit/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGPATCHES :=
PKGCFG      = --host=$(CROSS_TARGET) --prefix= --disable-ipcalc-symlink
PKGINSTALL := install-exec

include $(ROOTDIR)/system/pkg.mk
