# Glue for cross compiling Dropbear SSH
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

VER            ?= 2014.66
PKG             = dropbear-$(VER)
PKGSUFFIX       = tar.bz2
PATCHES         = patches/*.patch
URL             = http://matt.ucc.asn.au/dropbear/releases/$(PKG).$(PKGSUFFIX)
EXEC            = $(PKG)/dropbearmulti
CFLAGS         := $(filter-out -W -Wall -Werror, $(CFLAGS))

CFG_HOST        = --host=$(CROSS_COMPILE:-=) --prefix= --sysconfdir=/mnt
CFG_OPTS        = --disable-wtmp
CFG_OPTS       += --disable-shadow --disable-lastlog

ENV_OPTS        = PROGRAMS="dropbear dropbearkey dbclient scp" MULTI=1 SCPPROGRESS=1

all: build

include $(ROOTDIR)/pkg.mk

$(PKG)/.prepared: Makefile $(ARCHIVE) $(PATCHES) patches/series
	-@$(RM) -r $(PKG)
	@tar xfj $(ARCHIVE)
	@echo "  PATCH   $(PKG)/"
	@(cd $(PKG); ln -sf ../patches .)
	@(cd $(PKG); quilt push -qa)
	@touch $@

unpack: $(PKG)/.prepared

$(PKG)/.stamp: $(PKG)/.prepared
	@(cd $(PKG) && export ac_cv_member_struct_utmp_ut_host=yes; export ac_cv_member_struct_utmp_ut_pid=yes; export ac_cv_member_struct_utmp_ut_type=yes; ./configure $(CFG_HOST) $(CFG_OPTS))
	@touch $@

$(EXEC): $(PKG)/.stamp
	@$(MAKE) -C $(PKG) $(ENV_OPTS)
	@echo "Built $(EXEC) OK."

build: $(EXEC)

# The paths are hard wired in the dropbear ssh deamon, so please don't
# try to move these binaries around.
install:
	@mkdir -p $(STAGING)/etc/dropbear
	@cp -v $(EXEC) $(STAGING)/sbin/dropbear
	@(cd $(STAGING);					\
		ln -sfv    ../sbin/dropbear bin/dropbearkey;	\
		ln -sfv ../../sbin/dropbear usr/bin/dbclient;	\
		ln -sfv ../../sbin/dropbear usr/bin/ssh;	\
		ln -sfv ../../sbin/dropbear usr/bin/scp)

clean:
	-@$(MAKE) -C $(PKG) clean

distclean:
	-@$(RM) -rf dropbear-*

