# Glue for cross compiling the SSDP discovery agent ssdp-responder
PKGVER     := 1.5
PKG        := ssdp-responder-$(PKGVER)
PKGSUFFIX  := tar.gz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGCFG      = --host=$(CROSS_TARGET) --prefix=
PKGPATCHES :=

include $(ROOTDIR)/system/pkg.mk
