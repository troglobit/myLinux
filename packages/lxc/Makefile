# Glue for cross compiling netcalc
PKGVER     := 2.1.1
PKG        := lxc-$(PKGVER)
PKGURL      = https://linuxcontainers.org/downloads/lxc/$(PKGTAR)
PKGPATCHES :=
PKGCFG      = --host=$(CROSS_TARGET) --prefix= --with-distro=troglos
PKGCFG     += --disable-apparmor --disable-seccomp --disable-selinux
PKGCFG     += --disable-werror --disable-bash --disable-gnutls
PKGCFG     += --disable-capabilities --disable-python --disable-lua

install::
	@cp vm $(STAGING)/sbin/

include $(ROOTDIR)/pkg.mk
