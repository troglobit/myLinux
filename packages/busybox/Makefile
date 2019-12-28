# Glue for cross compiling BusyBox
include $(ROOTDIR)/core.mk

PKGVER     := 1.29.3
PKG	   := busybox-$(PKGVER)
PKGSUFFIX  := tar.bz2
PKGURL      = http://busybox.net/downloads/$(PKGTAR)
PKGENV     := CONFIG_PREFIX=$(STAGING)

include $(ROOTDIR)/pkg.mk

BASE_VER   := $(shell echo $(PKGVER) | sed 's/\([0-9]*\.[0-9]*\)\.[0-9]*/\1/')
CONFIG     := config-$(BASE_VER)

$(PKG)/.config:: $(PKG)/.unpacked
	@if [ ! -f $(CONFIG) ]; then					\
		echo "Warning: .config missing for $(PKG)";		\
		echo "         You need to create it from scratch.";	\
		exit 1;							\
	fi
	@if [ ! -s $(PKG)/.config ]; then				\
		cp $(CONFIG) $@;					\
	fi

saveconfig:
	@echo -n "  SAVING  "
	@cp -v $(PKG)/.config $(CONFIG)

menuconfig: oldconfig
	@$(MAKE) -C $(PKG) menuconfig

install::
	@cp -v $(PKG)/examples/udhcp/udhcpd.conf $(STAGING)/etc/

