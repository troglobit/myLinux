# Glue for cross compiling Inadyn DDNS client
PKGVER     := 2.3
PKG        := inadyn-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGCFG      = --host=$(CROSS_TARGET) --prefix= --disable-ssl
PKGINSTALL := install-exec

include $(ROOTDIR)/system/pkg.mk

