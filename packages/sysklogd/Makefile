# Glue for cross compiling sysklogd
PKGVER     := 2.0.3
PKG        := sysklogd-$(PKGVER)
PKGSUFFIX  := tar.gz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGCFG      = --host=$(CROSS_TARGET) --prefix= --without-klogd
PKGINSTALL := install-exec

install::
	@cp syslog.conf $(STAGING)/etc/

include $(ROOTDIR)/pkg.mk
