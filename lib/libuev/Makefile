# Glue for cross compiling libuEv
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

VER            ?= 1.2.3
NAME            = libuev
PKG		= $(NAME)-$(VER)
PKGSUFFIX	= tar.xz
PATCHES 	= patches/*.patch
URL             = https://github.com/troglobit/$(NAME)/releases/download/v$(VER)/$(PKG).$(PKGSUFFIX)

all: build

include $(ROOTDIR)/pkg.mk

$(PKG)/Makefile: $(ARCHIVE) $(PATCHES) patches/series
	-@$(RM) -r $(PKG)
	@tar xfJ $(ARCHIVE)
	@echo "  PATCH   $(PKG)/"
	@(cd $(PKG); ln -sf ../patches .)
	@(cd $(PKG); quilt push -qa)
	@touch $@

unpack: $(PKG)/Makefile

$(PKG)/.stamp: $(PKG)/Makefile
	+@$(MAKE) -C $(PKG)
	@touch $@

build: $(PKG)/.stamp

install:
	+@$(MAKE) DESTDIR=$(STAGING) prefix= -C $(PKG) install

clean:
	-@$(MAKE) -C $(PKG) clean
	-@$(RM) $(PKG)/.stamp

distclean:
	-@$(RM) -rf $(NAME)-*

