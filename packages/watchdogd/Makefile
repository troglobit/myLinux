# Glue for cross compiling watchdogd

PKGVER     := 2.0
PKG        := watchdogd-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/$(PKGVER)/$(PKGTAR)
PKGINSTALL := install-exec

include $(ROOTDIR)/pkg.mk

$(PKG)/.stamp::
	@$(MAKE) $(PKGENV) -C $(PKG) install-dev
