# Glue for cross compiling Merecat httpd
include $(ROOTDIR)/core.mk

PKGVER     := 2.31
PKG        := merecat-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGPATCHES :=
PKGCFG     := --host=$(CROSS_TARGET) --prefix=
PKGCFG     += --prefix=/usr --localstatedir=/var --sysconfdir=/etc
PKGCFG     += --without-ssl
PKGINSTALL := install

include $(ROOTDIR)/pkg.mk
