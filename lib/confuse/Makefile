# Glue for cross compiling libConfuse, a .conf file parser library
PKGVER     := 3.0
PKG        := confuse-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = https://github.com/martinh/libconfuse/releases/download/v$(PKGVER)/$(PKGTAR)
PKGCFG      = --host=$(CROSS_TARGET) --prefix= --disable-examples

include $(ROOTDIR)/system/pkg.mk
