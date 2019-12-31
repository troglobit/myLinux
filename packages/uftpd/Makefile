# Glue for cross compiling uftpd
PKGVER     := 2.8
PKG        := uftpd-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGPATCHES :=
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGCFG      = --prefix= --host=$(CROSS_TARGET)
PKGINSTALL := install-exec

include $(ROOTDIR)/pkg.mk
