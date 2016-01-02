# Glue for cross compiling Inadyn DDNS client

PKGVER     := 1.99.15
PKG        := inadyn-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/$(PKGVER)/$(PKGTAR)
PKGCFG     := --host=$(CROSS_TARGET) --prefix= --disable-ssl
PKGINSTALL := install-exec

include $(ROOTDIR)/pkg.mk

