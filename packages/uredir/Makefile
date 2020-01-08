# Glue for cross compiling the UDP redirector uredir
PKGVER     := 3.2
PKG        := uredir-$(PKGVER)
PKGSUFFIX  := tar.gz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGCFG      = --host=$(CROSS_TARGET) --prefix=
PKGPATCHES :=

include $(ROOTDIR)/system/pkg.mk
