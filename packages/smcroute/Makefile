# Glue for cross compiling SMCRoute

PKGVER     := 2.2.2
PKG        := smcroute-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/$(PKGVER)/$(PKGTAR)
PKGPATCHES :=
PKGCFG     := --host=$(CROSS_TARGET) --prefix= --without-libcap
PKGINSTALL := install-exec

include $(ROOTDIR)/pkg.mk
