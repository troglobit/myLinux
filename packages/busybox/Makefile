# Glue for cross compiling BusyBox
#
# Copyright (c) 2013-2014  Henrik Nordström <henrik@henriknordstrom.net>
# Copyright (c) 2014-2015  Joachim Nilsson <troglobit@gmail.com
#
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted, provided that the
# above copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
# IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
.PHONY: all install clean distclean

VER            ?= 1.23.0
BASE_VER       := $(shell echo $(VER) | sed 's/\([0-9]*\.[0-9]*\)\.[0-9]*/\1/')
PKG		= busybox-$(VER)
PKGSUFFIX	= tar.bz2
PATCHES 	= patches/*.patch
URL             = http://busybox.net/downloads/$(PKG).$(PKGSUFFIX)
CONFIG		= config-$(BASE_VER)
CFLAGS         := $(filter-out -W -Wall, $(CFLAGS))
CFLAGS         := $(filter-out -Werror, $(CFLAGS))
CFLAGS         := $(filter-out -Wcast-align, $(CFLAGS))
CFLAGS         := $(filter-out -Wformat, $(CFLAGS))

all: build

include $(ROOTDIR)/pkg.mk

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
	@echo -n "  SAVING  "
	@cp -v $(PKG)/.config $(CONFIG)

menuconfig: oldconfig
	@$(MAKE) -C $(PKG) menuconfig

$(PKG)/.stamp: $(PKG)/.config $(CONFIG)
	+@$(MAKE) -C $(PKG) busybox
	@touch $@

build: $(PKG)/.stamp

install:
	+@$(MAKE) -C $(PKG) CONFIG_PREFIX=$(STAGING) install
	@cp -v $(PKG)/examples/udhcp/udhcpd.conf $(STAGING)/etc/

clean:
	-@$(MAKE) -C $(PKG) clean
	-@$(RM) $(PKG)/.stamp

distclean:
	-@$(RM) -rf busybox-*

