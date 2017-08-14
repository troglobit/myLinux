# Glue for cross compiling Das U-Boot
include $(ROOTDIR)/core.mk

PKGVER     := 20170703
PKGNAME    := rpi-firmware
PKG	   := firmware-1.$(PKGVER)
PKGSUFFIX  := tar.gz
PKGTAR     := 1.$(PKGVER).$(PKGSUFFIX)
PKGURL      = https://github.com/raspberrypi/firmware/archive/$(PKGTAR)
PKGPATCHES :=

include $(ROOTDIR)/pkg.mk

$(PKG)/Makefile: wrap.mk $(PKG)/.unpacked
	@echo "  STRAP   $@"
	@cp -vf $< $@

$(PKG)/.config:: $(PKG)/Makefile
