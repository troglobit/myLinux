# Glue for cross compiling nemesis
PKGVER     := 1.6
PKG        := nemesis-$(PKGVER)
PKGSUFFIX  := tar.gz
PKGURL      = https://github.com/troglobit/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGPATCHES :=
PKGCFG      = --host=$(CROSS_TARGET) --prefix=
PKGINSTALL := install-exec

include $(ROOTDIR)/system/pkg.mk

install::
	@ln -sf mg $(STAGING)/bin/emacs
