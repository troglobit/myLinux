# Glue for cross compiling BusyBox
VER            ?= 1.21.0
PKG		= busybox-$(VER)
PKGSUFFIX	= tar.bz2
PATCHES 	= patches/*.patch
URL             = http://busybox.net/downloads/$(PKG).$(PKGSUFFIX)
CONFIG		= $(PKG).config
CFLAGS         := $(filter-out -W -Wall, $(CFLAGS))
CFLAGS         := $(filter-out -Werror, $(CFLAGS))
CFLAGS         := $(filter-out -Wcast-align, $(CFLAGS))
CFLAGS         := $(filter-out -Wformat, $(CFLAGS))

all: build

include $(ROOTDIR)/package.mk

$(PKG)/Makefile: $(ARCHIVE) $(PATCHES) patches/series
	-@$(RM) -r $(PKG)
	@tar xfj $(ARCHIVE)
	@echo "  PATCH   $(PKG)/"
	@(cd $(PKG); ln -sf ../patches .)
	@(cd $(PKG); quilt push -qa)
	@touch $@

unpack: $(PKG)/Makefile

$(PKG)/.config: $(PKG)/Makefile
	@if [ -f $(CONFIG) ]; then					\
		cp $(CONFIG) $@;					\
	else								\
		echo "Warning: .config missing for $(PKG)";		\
		echo "         You need to create it from scratch.";	\
		exit 1;							\
	fi

oldconfig: $(PKG)/.config

saveconfig:
	@cp -v $(PKG)/.config $(CONFIG)

menuconfig: oldconfig
	@$(MAKE) -C $(PKG) menuconfig

$(PKG)/.stamp: $(PKG)/.config $(CONFIG)
	+@$(MAKE) -C $(PKG) busybox
	@touch $@

build: $(PKG)/.stamp

install:
	+@$(MAKE) -C $(PKG) CONFIG_PREFIX=$(STAGING) install
	@$(CP) $(PKG)/examples/udhcp/udhcpd.conf $(STAGING)/etc/

clean:
	-@$(MAKE) -C $(PKG) clean
	-@$(RM) $(PKG)/.stamp

distclean:
	-@$(RM) busybox-*

