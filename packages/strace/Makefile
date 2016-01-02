# Glue for cross compiling strace

PKGVER     := 4.11
PKG        := strace-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = http://sourceforge.net/projects/$(PKGNAME)/files/$(PKGNAME)/$(PKGVER)/$(PKGTAR)
PKGPATCHES :=
PKGCFG     := --host=$(CROSS_TARGET) --prefix=
PKGINSTALL := install-exec

include $(ROOTDIR)/pkg.mk

