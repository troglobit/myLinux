# Glue for cross compiling the UDP redirector uredir

PKGVER     := 2.0
PKG        := uredir-$(PKGVER)
PKGSUFFIX  := tar.gz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGCFG     := --host=$(CROSS_TARGET) --prefix=
PKGPATCHES :=
PKGINSTALL := install

include $(ROOTDIR)/pkg.mk
