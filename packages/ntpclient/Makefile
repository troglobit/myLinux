# Glue for cross compiling netcalc
PKGVER     := 2017_246
PKG        := ntpclient-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = https://github.com/troglobit/$(PKGNAME)/releases/download/$(PKGVER)/$(PKGTAR)
PKGPATCHES :=
PKGCFG      = --host=$(CROSS_TARGET) --prefix=
PKGINSTALL := install-exec

include $(ROOTDIR)/system/pkg.mk
