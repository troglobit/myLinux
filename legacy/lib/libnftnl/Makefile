# Glue for cross compiling libnftl, low-level API to Linux nf_tables
PKGVER     := 1.1.5
PKG        := libnftnl-$(PKGVER)
PKGSUFFIX  := tar.bz2
PKGURL      = http://ftp.netfilter.org/pub/libnftnl/$(PKGTAR)
PKGCFG      = --host=$(CROSS_TARGET) --prefix=$(STAGING)

include $(ROOTDIR)/system/pkg.mk
