# Glue for cross compiling the UDP redirector uredir
include $(ROOTDIR)/core.mk

PKGVER     := 3.1
PKG        := uredir-$(PKGVER)
PKGSUFFIX  := tar.gz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGCFG     := --host=$(CROSS_TARGET) --prefix=
PKGPATCHES :=
PKGINSTALL := install

include $(ROOTDIR)/pkg.mk
