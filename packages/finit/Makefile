# Glue for cross compiling Finit SysV init replacement
#
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

#VER            ?= 1.10
VER             = dev
VERSION         = $(VER)
NAME            = finit
PKG		= $(NAME)-$(VER)
PKGSUFFIX	= tar.xz
PATCHES 	= patches/*.patch
URL             = https://github.com/troglobit/$(NAME)/releases/download/$(VER)/$(PKG).$(PKGSUFFIX)

CPPFLAGS        = -DEMBEDDED_SYSTEM
PLUGINS         = bootmisc.so initctl.so hwclock.so time.so
DESTDIR         = $(STAGING)

export CPPFLAGS PLUGINS DESTDIR


all: build

include $(ROOTDIR)/pkg.mk

$(PKG)/Makefile: $(ARCHIVE) $(PATCHES) patches/series
	-@$(RM) -r $(PKG)
	@tar xfJ $(ARCHIVE)
	@echo "  PATCH   $(PKG)/"
	@(cd $(PKG); rm -rf patches; ln -sf ../patches .)
	@(cd $(PKG); quilt push -qa)
	@touch $@

unpack: $(PKG)/Makefile

$(PKG)/.stamp: $(PKG)/Makefile
	+@$(MAKE) -C $(PKG)
	@touch $@

build: $(PKG)/.stamp

install:
	+@$(MAKE) -C $(PKG) install-exec
	@ln -sf sbin/finit $(STAGING)/init

clean:
	-@$(MAKE) -C $(PKG) clean
	-@$(RM) $(PKG)/.stamp

distclean:
	-@$(RM) -rf finit-*

