# Glue for cross compiling netcalc
PKGVER     := 3.2.1
PKG        := lxc-$(PKGVER)
PKGURL      = https://linuxcontainers.org/downloads/lxc/$(PKGTAR)
PKGPATCHES :=
PKGCFG      = --host=$(CROSS_TARGET) --prefix= --with-distro=myLinux
PKGCFG     += --disable-apparmor --disable-seccomp --disable-selinux
PKGCFG     += --disable-werror --disable-bash --disable-gnutls
PKGCFG     += --disable-capabilities --disable-python --disable-lua
PKGCFG     += --disable-api-docs

include $(ROOTDIR)/system/pkg.mk

# Required extra step due to no-werror-all.patch
$(PKG)/.unpacked::
	@(cd $(PKG)/ && ./autogen.sh)

install::
	@rm -rf $(STAGING)/share/lxc/config $(STAGING)/share/lxc/templates
	@cp cx $(STAGING)/sbin/
