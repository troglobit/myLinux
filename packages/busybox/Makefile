# Glue for cross compiling BusyBox

PKGVER     := 1.24.1
PKG	   := busybox-$(PKGVER)
PKGSUFFIX  := tar.bz2
PKGURL      = http://busybox.net/downloads/$(PKGTAR)
PKGENV     := CONFIG_PREFIX=$(STAGING)

include $(ROOTDIR)/.config
include $(ROOTDIR)/pkg.mk

BASE_VER   := $(shell echo $(PKGVER) | sed 's/\([0-9]*\.[0-9]*\)\.[0-9]*/\1/')
CONFIG     := config-$(BASE_VER)

$(PKG)/.config::
	@if [ -f $(CONFIG) ]; then					\
		cp $(CONFIG) $@;					\
	else								\
		echo "Warning: .config missing for $(PKG)";		\
		echo "         You need to create it from scratch.";	\
		exit 1;							\
	fi

saveconfig:
	@echo -n "  SAVING  "
	@cp -v $(PKG)/.config $(CONFIG)

menuconfig: oldconfig
	@$(MAKE) -C $(PKG) menuconfig

install::
ifdef CONFIG_FINIT
	@echo "  REMOVE  $(STAGING)/sbin/init due to Finit"
	@rm $(STAGING)/sbin/init
endif
	@cp -v $(PKG)/examples/udhcp/udhcpd.conf $(STAGING)/etc/ $(REDIRECT)

