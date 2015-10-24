# Glue for cross compiling lldpd, IEEE 802.1AB
#
# Copyright (c) 2015  Joachim Nilsson <troglobit@gmail.com
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

VER            ?= 0.7.11
#VER             = dev
NAME            = lldpd
PKG		= $(NAME)-$(VER)
PKGSUFFIX       = tar.gz
PATCHES         = patches/*.patch
URL             = http://media.luffy.cx/files/lldpd/$(PKG).$(PKGSUFFIX)
CFG_OPTS        = --prefix=/ --host=arm-linux-gnueabi
CFG_OPTS       += --with-privsep-user=lldpd
CFG_OPTS       += --with-privsep-group=lldpd
CFG_OPTS       += --with-privsep-chroot=/var/run
CFG_OPTS       += --enable-cdp
CFG_OPTS       += --enable-lldpmed
CFG_OPTS       += --enable-dot1 --enable-dot3
CFG_OPTS       += --disable-fdp
CFG_OPTS       += --disable-edp --disable-sonmp
#CFG_OPTS       += --with-snmp

all: build

include $(ROOTDIR)/pkg.mk

$(PKG)/Makefile: $(ARCHIVE) $(PATCHES) patches/series
	-@$(RM) -r $(PKG)
	@tar xfz $(ARCHIVE)
	@echo "  PATCH   $(PKG)/"
	@(cd $(PKG);				\
		ln -sf ../patches .;		\
		quilt push -qa;			\
		./configure $(CFG_OPTS))
	@touch $@

unpack: $(PKG)/Makefile

$(PKG)/.stamp: $(PKG)/Makefile
	+@$(MAKE) -C $(PKG)
	@touch $@

build: $(PKG)/.stamp

install:
	+@$(MAKE) DESTDIR=$(STAGING) -C $(PKG) install-exec

clean:
	-@$(MAKE) -C $(PKG) clean
	-@$(RM) $(PKG)/.stamp

distclean:
	-@$(RM) -rf $(NAME)-*

