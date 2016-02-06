# Glue for cross compiling uftpd

PKGVER     := 2.0.2
PKG        := uftpd-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGPATCHES :=
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGCFG     := --prefix= --host=$(CROSS_TARGET)
#PKGCFG     += --with-libuev=$(STAGING) --with-libite=$(STAGING)
PKGINSTALL := install-exec

include $(ROOTDIR)/pkg.mk
