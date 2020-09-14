# Glue for cross compiling iptables
PKGVER     := 1.8.4
PKG        := iptables-$(PKGVER)
PKGSUFFIX  := tar.bz2
PKGURL      = http://ftp.netfilter.org/pub/$(PKGNAME)/$(PKGTAR)
PKGCFG      = --host=$(CROSS_TARGET) --prefix=
PKGCFG     += 
PKGINSTALL := install

include $(ROOTDIR)/system/pkg.mk
