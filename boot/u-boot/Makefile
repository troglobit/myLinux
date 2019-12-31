# Glue for cross compiling Das U-Boot
PKGVER     := 2017.07
PKG	   := u-boot-$(PKGVER)
PKGSUFFIX  := tar.bz2
PKGURL      = ftp://ftp.denx.de/pub/u-boot/$(PKGTAR)
PKGENV     := CROSS_COMPILE=$(CROSS_COMPILE) ARCH=$(ARCH)

include $(ROOTDIR)/pkg.mk

$(PKG)/.config::
	@if [ ! -s $(PKG)/.config ]; then							\
		echo "  CONFIG  $(PKG)";							\
		(cd $(PKG) && make rpi_2_defconfig);						\
	else											\
		echo "  NO-OP   $(PKG)/.config already installed."				\
					| tee -a $(BUILDLOG); 					\
	fi

install::
	@cp -v $(PKG)/u-boot.bin $(IMAGEDIR)/ $(REDIRECT)
