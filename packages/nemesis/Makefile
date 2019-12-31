# Glue for cross compiling nemesis
PKGVER     := 1.6
PKG        := nemesis-$(PKGVER)
PKGSUFFIX  := tar.gz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGPATCHES :=
PKGCFG      = --host=$(CROSS_TARGET) --prefix=
PKGINSTALL := install-exec

install::
	@ln -sf mg $(STAGING)/bin/emacs

include $(ROOTDIR)/pkg.mk
