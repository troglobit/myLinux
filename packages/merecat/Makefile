# Glue for cross compiling Merecat httpd
PKGVER     := 2.31
PKG        := merecat-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = https://github.com/troglobit/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGPATCHES :=
PKGCFG      = --host=$(CROSS_TARGET) --prefix=
PKGCFG     += --prefix=/usr --localstatedir=/var --sysconfdir=/etc
PKGCFG     += --without-ssl

include $(ROOTDIR)/system/pkg.mk
