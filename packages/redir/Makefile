# Glue for cross compiling the TCP redirector redir
include $(ROOTDIR)/core.mk

PKGVER     := 3.1
PKG        := redir-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGCFG     := --host=$(CROSS_TARGET) --prefix=
PKGPATCHES :=
PKGINSTALL := install

include $(ROOTDIR)/pkg.mk
