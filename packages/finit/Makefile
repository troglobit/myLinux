# Glue for cross compiling Finit SysV init replacement

PKGVER     := 3.0-rc1
PKG        := finit-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/$(PKGVER)/$(PKGTAR)
PLUGINS    := bootmisc echo initctl hwclock tty time netlink urandom
PKGCFGENV  := CFLAGS="-W -Wall -Wextra"
PKGCFG     := --host=$(CROSS_TARGET) --prefix= --with-heading="$(OSPRETTY_NAME)"
PKGCFG     += --enable-inetd-echo-plugin --enable-inetd-time-plugin
PKGCFG     += --enable-fallback-shell
PKGINSTALL := install-exec

include $(ROOTDIR)/pkg.mk

$(PKG)/.stamp::
	@$(MAKE) $(PKGENV) -C $(PKG) install-dev $(REDIRECT)
	@$(MAKE) -C plugins

install::
	@ln -sf finit $(STAGING)/sbin/init
	@ln -sf sbin/init $(STAGING)/init
	@$(MAKE) -C plugins $@

clean::
	@$(MAKE) -C plugins $@

distclean::
	@$(MAKE) -C plugins $@

