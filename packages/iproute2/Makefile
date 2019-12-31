# Glue for cross compiling iproute2
PKGVER     := 5.2.0
PKG        := iproute2-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = https://www.kernel.org/pub/linux/utils/net/iproute2/$(PKGTAR)
PKGENV      = CPPFLAGS=-I$(STAGING)/include LDFLAGS="-L$(STAGING)/lib"
PKGENV     += DESTDIR=$(STAGING) PREFIX=/
PKGCFG      = --host=$(CROSS_TARGET) --prefix=

include $(ROOTDIR)/pkg.mk
