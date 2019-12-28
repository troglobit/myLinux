# Extensible build recipe for package/library/kernel
#
# Copyright (c) 2015-2017  Joachim Nilsson <troglobit@gmail.com
#
# Assumes PKG, PKGVER, and PKGURL being set in the including Makefile.
# See below for possibilities to override and extend both variables and
# rules, e.g. PKGNAME and install:: rule.
#
# NOTE: As long as "lazy evaluation" (=) is used, variables defined
#       in this file may be used before this file is included.
#
### Variables required by including Makefile
#
# PKG        - Full name of extracted package dir, e.g. uftpd-1.9.1
# PKGVER     - Full version string of package, e.g. 1.9.1
# PKGURL     - URL to PKG location, e.g. ftp://example.com/$(PKGTAR)
#
### Optional variables for including makefile
#
# PKGTARGETS - Override default build targets for packages, default: build
# PKGCFG     - Enable configure in pre-build step, used as args, e.g. --host
# PKGCFGENV  - Any environment variables required for configure, e.g. CFLAGS
# PKGBASEVER - Base version used for patches, e.g. 1.9
#              If set, build system will attempt to use patches/1.9/*.patch
# PKGPATCHES - Used only in Defaults to patches/*.patch, assumes Quilt is used
# PKGSUFFIX  - Defaults to tar.gz, may be overridden
# PKGDEVPATH - If set when calling the `dev` rule, this creates a developer
#              symlink in lib/$(PKGNAME)-name or packages/$(PKGNAME)-dev which
#              can be used to evaluate or develop packages between releases.
#
### Variables set by pkg.mk, may be used by including Makefile
#
# PKGNAME    - Basename without paths or version suffix, e.g. uftpd
#              Override if PKG is not of the form: PKGNAME-VER
# PKGENV     - Any environment variables requried for build and install,
#              defaults to: DESTDIR=$(STAGING) prefix=
# PKGTAR     - Derived from PKG and PKGSUFFIX, e.g. uftpd-1.9.1.tar.gz
#
### License
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

PKGFETCH   ?= wget -t3 -nc --no-dns-cache --no-iri -q -cO
PKGNAME    ?= $(PKG:-$(PKGVER)=)
PKGDEV     := $(PKGNAME)-dev
ifneq ("$(wildcard $(PKGDEV))","")
PKG        := $(PKGDEV)
endif
PKGSUFFIX  ?= tar.gz
PKGPATCHES ?= patches/*.patch
PKGTAR     ?= $(PKG).$(PKGSUFFIX)
PKGENV     ?= DESTDIR=$(STAGING) prefix=
PKGTARGETS ?= build
PKGINSTALL ?= install
PKGSUM      = $(PKGTAR).md5
PKGCKSUM    = $(shell pwd)/checksums/$(PKGSUM)
PKGTMP      = $(ROOTDIR)/tmp/$(PKGTAR)
PKGARCHIVE  = $(DOWNLOADS)/$(PKGNAME)/$(PKGTAR)
PKGMIRROR   = $(FTP_MIRROR)/$(PKGNAME)/$(PKGTAR)

all: $(PKGTARGETS)

$(PKGARCHIVE):
ifeq ("$(wildcard $(PKGDEV))","")
	@mkdir -p $(dir $@)
	@mkdir -p $(ROOTDIR)/tmp
	@if [ -e $(PKGTMP) ]; then						\
		echo "  WARNING Previous download failed, cleaning up ...";	\
		rm $(PKGTMP);							\
	fi
	@if [ x"$(FTP_MIRROR)" != x ]; then					\
		echo "  WGET    $(PKGMIRROR) ...";				\
		$(PKGFETCH) $(PKGTMP) $(PKGMIRROR);				\
	else									\
		echo "  WGET    $(PKGURL) ...";					\
		$(PKGFETCH) $(PKGTMP) $(PKGURL);				\
	fi
	@mv $(PKGTMP) $@
	@if [ $$? -ne 0 ]; then							\
		rm $(PKGTMP);							\
		exit 1;								\
	fi
else
	@true
endif

download: $(PKGARCHIVE)

$(PKGCKSUM): $(PKGARCHIVE)
	@echo "  CHKSUM  $(PKGTAR)"
	@mkdir -p checksums
	@(cd $(dir $(PKGARCHIVE)); md5sum $(PKGTAR)) > $@
	@git add -f $@

chksum: $(PKGCKSUM)

ifeq ("$(wildcard $(PKGDEV))","")
$(PKG)/.unpacked:: Makefile $(PKGARCHIVE) $(PKGPATCHES)
	-@$(RM) -r $(PKG)
	@(cd $(dir $(PKGARCHIVE));						\
	  md5sum -c $(PKGCKSUM)	2>&1 >/dev/null					\
	  || { echo "  FAILED  Verifying $(PKGTAR) checksum!"; exit 1; })
	@echo "  VRFY    $(PKG) checksum OK"
	@echo "  UNPACK  $(PKG)"
	@tar xf $(PKGARCHIVE)
	@if [ -d patches/$(PKGBASEVER) ]; then					\
		echo "  PATCH   $(PKG)";					\
		(cd $(PKG);							\
		 ln -sf ../patches/$(PKGBASEVER) patches;			\
		 quilt push -qa);						\
	fi
else
$(PKG)/.unpacked:: Makefile
	-@rm $(PKG)/.config
endif
	@touch $@

unpack: $(PKG)/.unpacked

# Default rule, override with your own to create Makefile for build step
# Silly test -s is to check that $PKGCFG is set ...
$(PKG)/.config:: $(PKG)/.unpacked
	@tmpfile=`mktemp /tmp/troglos.XXXXXX)`;					\
	echo -n '$(PKGCFG)' > $$tmpfile;					\
	if [ -s $$tmpfile ]; then						\
		rm $$tmpfile;							\
		cd $(PKG);							\
		if [ ! -e configure -a -e autogen.sh ]; then			\
			echo "  AUTOGEN $(PKG)";				\
			./autogen.sh;						\
		fi;								\
		echo "  CONFIG  $(PKG)";					\
		$(PKGCFGENV) ./configure $(PKGCFG);				\
		cd - >/dev/null;						\
	else									\
		rm $$tmpfile;							\
	fi
	@touch $@

oldconfig:: $(PKG)/.config

$(PKGDEV):
	if [ -z "$(PKGDEVPATH)" ]; then						\
		echo "  ERROR   Cannot set up $(PKGDEV), missing PKGDEVPATH!";	\
		echo "          Example: PKGDEVPATH=/path/to/pkg-vcs";		\
		exit 1;								\
	fi
	@if [ ! -d "$(PKGDEVPATH)" ]; then					\
		echo "	ERROR	PKGDEVPATH=$(PKGDEVPATH) is not a directory!";	\
		exit 1;								\
	fi
	@ln -sf $(PKGDEVPATH) $(PKGDEV)
	@echo "  DEV     Developer mode enabled for $(PKGNAME)"

dev: $(PKGDEV)

$(PKG)/.stamp:: $(PKG)/.config
	@echo "  BUILD   $(PKG)"
	+@$(MAKE) $(PKGENV) -C $(PKG)
ifeq ("$(wildcard $(PKGDEV))","")
	@touch $@
endif

ifeq ("$(wildcard $(PKGDEV))","")
build: $(PKG)/.stamp
else
check:
	-@if [ ! -e $(PKG)/configure -o ! -e $(PKG)/Makefile ]; then		\
		rm $(PKG)/.stamp $(PKG)/.config $(PKG)/.unpacked 2>/dev/null;	\
	fi
build: check $(PKG)/.stamp
endif

clean::
	-@$(MAKE) -C $(PKG) clean
	-@$(RM) $(PKG)/.stamp

distclean::
ifeq ("$(wildcard $(PKGDEV))","")
	-@$(RM) -rf $(PKGNAME)-*
else
	@unset CLEAN_DIRS
	-@$(RM) $(PKG)/.stamp $(PKG)/.config
	-@$(MAKE) -C $(PKG) $@
endif

install:: build
	@echo "  INSTALL $(PKG)"
	+@$(MAKE) $(PKGENV) -C $(PKG) $(PKGINSTALL)
ifdef CONFIG_FINIT
	@mkdir -p $(FINIT_D_AVAILABLE)
	@for file in $(wildcard finit.d/*.conf); do 				\
		echo "  INSTALL $(PKG)/$$file $(FINIT_D_AVAILABLE)/";		\
		cp $$file $(FINIT_D_AVAILABLE)/; 				\
	done
endif
