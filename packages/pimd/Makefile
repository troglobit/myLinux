# Glue for cross compiling pimd
include $(ROOTDIR)/core.mk

PKGVER     := 2.3.2
PKG        := pimd-$(PKGVER)
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/$(PKGVER)/$(PKGTAR)
PKGPATCHES :=
PKGCFG     := --prefix= --enable-rsrr
CC         := gcc

include $(ROOTDIR)/pkg.mk
