# Glue for cross compiling sysklogd
PKGVER     := 2.1.1
PKG        := sysklogd-$(PKGVER)
PKGSUFFIX  := tar.gz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGCFG      = --host=$(CROSS_TARGET) --prefix=
PKGINSTALL := install-exec

include $(ROOTDIR)/system/pkg.mk

install::
	@cp syslog.conf $(STAGING)/etc/
