# Glue for cross compiling Finit SysV init replacement

PKGVER     := 2.4
PKG        := finit-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/$(PKGVER)/$(PKGTAR)
PLUGINS    := bootmisc initctl hwclock tty time netlink urandom
# --enable-static
PKGCFG     := --enable-embedded
PKGCFG     += --with-heading="$(OSPRETTY_NAME)"
PKGCFG     += --with-plugins="$(PLUGINS)"
#PKGCFG     += --with-randomseed=/mnt/random-seed
PKGCFG     += --with-libite=$(STAGING)
PKGCFG     += --with-libuev=$(STAGING)
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

