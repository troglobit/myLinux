# Glue for cross compiling nftables
PKGVER     := 0.9.3
PKG        := nftables-$(PKGVER)
PKGSUFFIX  := tar.bz2
PKGURL      = http://ftp.netfilter.org/pub/$(PKGNAME)/$(PKGTAR)
PKGCFG      = --host=$(CROSS_TARGET) --prefix=
PKGCFG     += --disable-man-doc --without-cli --with-mini-gmp --disable-python
PKGCFGENV   = LT_SYS_LIBRARY_PATH=$(STAGING)/lib
PKGINSTALL := install-exec

include $(ROOTDIR)/system/pkg.mk

# Required due to explicit-lib-deps.patch
$(PKG)/.unpacked::
	@(cd $(PKG)/ && autoreconf -W portability -visfm)
