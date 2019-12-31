# Glue for cross compiling netsniff-ng
PKGVER     := 0.6.6
PKG        := netsniff-ng-$(PKGVER)
PKGSUFFIX  := tar.gz
PKGURL      = https://github.com/netsniff-ng/netsniff-ng/archive/v$(PKGVER).$(PKGSUFFIX)
PKGCFG      = --prefix= --disable-libnl --disable-geoip --disable-zlib
PKG_CONFIG := pkg-config

include $(ROOTDIR)/pkg.mk
