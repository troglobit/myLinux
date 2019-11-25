# Glue for cross compiling sysklogd
include $(ROOTDIR)/core.mk

PKGVER     := 2.0.1
PKG        := sysklogd-$(PKGVER)
PKGSUFFIX  := tar.gz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGCFG     := --host=$(CROSS_TARGET) --prefix=
PKGINSTALL := install-exec

include $(ROOTDIR)/pkg.mk

install::
	@cp syslog.conf $(STAGING)/etc/
