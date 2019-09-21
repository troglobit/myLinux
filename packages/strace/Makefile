# Glue for cross compiling strace
include $(ROOTDIR)/core.mk

PKGVER     := 4.23
PKG        := strace-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = https://strace.io/files/$(PKGVER)/$(PKGTAR)
PKGPATCHES :=
PKGCFG     := --host=$(CROSS_TARGET) --prefix= --without-libunwind
PKGINSTALL := install-exec

include $(ROOTDIR)/pkg.mk

install::
	@rm $(STAGING)/bin/strace-graph
