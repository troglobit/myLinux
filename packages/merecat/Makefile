# Glue for cross compiling Merecat httpd

PKGVER     := 2.30
PKG        := merecat-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGPATCHES :=
PKGCFG     := --host=$(CROSS_TARGET) --prefix=
PKGCFG     += --prefix=/usr --localstatedir=/var --sysconfdir=/etc
PKGINSTALL := install

include $(ROOTDIR)/pkg.mk
