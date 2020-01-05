# Glue for cross compiling watchdogd
PKGVER     := 3.3
PKG        := watchdogd-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/$(PKGVER)/$(PKGTAR)
PKGCFG      = --host=$(CROSS_TARGET) --prefix=
PKGCFG     += --disable-test-mode --enable-examples
PKGCFG     += --enable-loadavg --enable-filenr --enable-meminfo --enable-syslog-mark
PKGINSTALL := install-exec

include $(ROOTDIR)/pkg.mk

$(PKG)/.stamp::
	+@$(MAKE) $(PKGENV) -C $(PKG) install-dev

install::
	@cp watchdogd.conf $(STAGING)/etc/
