# Glue for cross compiling sntpd
PKGVER     := 3.0
PKG        := sntpd-$(PKGVER)
PKGSUFFIX  := tar.gz
PKGURL      = https://github.com/troglobit/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGPATCHES :=
PKGCFG      = --host=$(CROSS_TARGET) --prefix=
PKGINSTALL := install-exec

include $(ROOTDIR)/system/pkg.mk
