# Glue for cross compiling strace
PKGVER     := 5.2
PKG        := strace-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = https://strace.io/files/$(PKGVER)/$(PKGTAR)
PKGPATCHES :=
PKGCFG      = --host=$(CROSS_TARGET) --prefix=
PKGCFG     += --without-libunwind
PKGINSTALL := install-exec

include $(ROOTDIR)/system/pkg.mk

install::
	@rm $(STAGING)/bin/strace-graph
