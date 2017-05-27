# Glue for cross compiling Inadyn DDNS client
include $(ROOTDIR)/core.mk

PKGVER     := 2.0
PKG        := inadyn-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGCFG     := --host=$(CROSS_TARGET) --prefix= --disable-ssl
PKGINSTALL := install-exec

include $(ROOTDIR)/pkg.mk

