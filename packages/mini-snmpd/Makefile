# Glue for cross compiling Mini SNMP Daemon
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

VER            ?= 1.2b
NAME            = mini_snmpd
PKG		= $(NAME)-$(VER)
PKGSUFFIX	= tar.gz
PATCHES 	= patches/*.patch
URL             = http://members.aon.at/linuxfreak/linux/$(PKG).$(PKGSUFFIX)
CFLAGS         := $(filter-out -W -Wall, $(CFLAGS))
CFLAGS         := $(filter-out -Werror, $(CFLAGS))
CFLAGS         := $(filter-out -Wcast-align, $(CFLAGS))
CFLAGS         := $(filter-out -Wformat, $(CFLAGS))

all: build

include $(ROOTDIR)/pkg.mk

$(PKG)/Makefile: $(ARCHIVE) $(PATCHES) patches/series
	-@$(RM) -r $(PKG)
	@tar xfz $(ARCHIVE) && mv $(NAME) $(PKG)
	@echo "  PATCH   $(PKG)/"
	@(cd $(PKG); ln -sf ../patches .)
	@(cd $(PKG); quilt push -qa)
	@touch $@

unpack: $(PKG)/Makefile

$(PKG)/.stamp: $(PKG)/Makefile
	+@$(MAKE) -C $(PKG) OFLAGS="-O2 -D__IPV4__ -static" CC=$(CC)
	@touch $@

build: $(PKG)/.stamp

install:
	@$(CROSS_COMPILE)strip -o $(STAGING)/sbin/$(NAME) $(PKG)/$(NAME)

clean:
	-@$(MAKE) -C $(PKG) clean
	-@$(RM) $(PKG)/.stamp

distclean:
	-@$(RM) -rf mini_snmpd-*

