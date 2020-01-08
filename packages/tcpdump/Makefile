# Glue for cross compiling tcpdump
PKGVER     := 4.6.2
PKG        := tcpdump-$(PKGVER)
PKGSUFFIX  := tar.gz
PKGURL      = http://www.tcpdump.org/release/$(PKGTAR)
PKGCFGENV  += export ac_cv_linux_vers=2 td_cv_buggygetaddrinfo=no;
PKGCFG      = --host=$(CROSS_TARGET) --prefix= --without-smi
PKGCFG     += --disable-smb --without-crypto

include $(ROOTDIR)/system/pkg.mk
