# Glue for cross compiling the TCP redirector redir

PKGVER     := 3.0
PKG        := redir-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGCFG     := --host=$(CROSS_TARGET) --prefix=
PKGPATCHES :=
PKGINSTALL := install

include $(ROOTDIR)/pkg.mk
