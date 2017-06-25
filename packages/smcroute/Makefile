# Glue for cross compiling SMCRoute
include $(ROOTDIR)/core.mk

PKGVER     := 2.3.1
PKG        := smcroute-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/$(PKGVER)/$(PKGTAR)
PKGPATCHES :=
PKGCFG     := --host=$(CROSS_TARGET) --prefix=
PKGINSTALL := install-exec

include $(ROOTDIR)/pkg.mk
