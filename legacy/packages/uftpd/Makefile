# Glue for cross compiling uftpd
PKGVER     := 2.11
PKG        := uftpd-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGPATCHES :=
PKGURL      = https://github.com/troglobit/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGCFG      = --prefix= --host=$(CROSS_TARGET)
PKGINSTALL := install-exec

include $(ROOTDIR)/system/pkg.mk
