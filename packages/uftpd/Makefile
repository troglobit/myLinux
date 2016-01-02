# Glue for cross compiling uftpd

PKGVER     := 1.9.1
PKG        := uftpd-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGCFG     := --with-libuev=$(STAGING) --with-libite=$(STAGING)
PKGINSTALL := install-exec

include $(ROOTDIR)/pkg.mk
