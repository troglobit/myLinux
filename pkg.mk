# Extensible build recipe for package/library/kernel
#
# Copyright (c) 2015-2016  Joachim Nilsson <troglobit@gmail.com
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
TMPFILE     = $(ROOTDIR)/tmp/$(PKGTAR)
ARCHIVE     = $(DOWNLOADS)/$(PKGNAME)/$(PKGTAR)
MIRROR      = $(FTP_MIRROR)/$(PKGNAME)/$(PKGTAR)
tmpfile    := $(shell mktemp /tmp/troglos.XXXXXX)

all: $(PKGTARGETS)

$(ARCHIVE):
ifeq ("$(wildcard $(PKGDEV))","")
	@mkdir -p $(dir $@)
	@mkdir -p $(ROOTDIR)/tmp
	@if [ -e $(TMPFILE) ]; then						\
		echo "  WARNING Previous download failed, cleaning up ...";	\
		rm $(TMPFILE);							\
	fi
	@if [ x"$(FTP_MIRROR)" != x ]; then					\
		echo "  WGET    $(MIRROR) ...";					\
		$(PKGFETCH) $(TMPFILE) $(MIRROR);				\
	else									\
		echo "  WGET    $(PKGURL) ...";					\
		$(PKGFETCH) $(TMPFILE) $(PKGURL);				\
	fi
	@mv $(TMPFILE) $@
	@if [ $$? -ne 0 ]; then							\
		rm $(TMPFILE);							\
		exit 1;								\
	fi
else
	@true
endif

download: $(ARCHIVE)

checksums/$(PKG).sha1: $(ARCHIVE)
	@echo "Generating SHA1 checksum for $(PKG) ..."
	@mkdir -p checksums
	(cd $(dir $(ARCHIVE)); sha1sum $(PKG)*) > $@
	git add $@

gensum: checksums/$(PKG).sha1

$(PKG)/.unpacked:: Makefile $(ARCHIVE) $(PKGPATCHES)
ifeq ("$(wildcard $(PKGDEV))","")
	@echo "  UNPACK  $(PKG)"
	-@$(RM) -r $(PKG)
	@tar xf $(ARCHIVE)
	@if [ -d patches/$(PKGBASEVER) ]; then					\
		echo "  PATCH   $(PKG)";					\
		(cd $(PKG);							\
		 ln -sf ../patches/$(PKGBASEVER) patches;			\
		 quilt push -qa $(REDIRECT));					\
	fi
endif
	@touch $@

unpack: $(PKG)/.unpacked

# Dummy rule, override with your own call to ./configure or similar
$(PKG)/.config:: $(PKG)/.unpacked
	@echo -n '$(PKGCFG)' > $(tmpfile)
	@if [ -s $(tmpfile) ]; then						\
		rm $(tmpfile);							\
		echo "  CONFIG  $(PKG)";					\
		(cd $(PKG) && $(PKGCFGENV)					\
		  ./configure $(PKGCFG) $(REDIRECT));				\
	else									\
		rm $(tmpfile);							\
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
	+@$(MAKE) $(PKGENV) -C $(PKG) $(REDIRECT)
	@touch $@

build: $(PKG)/.stamp

clean::
	-@$(MAKE) -C $(PKG) clean $(REDIRECT)
	-@$(RM) $(PKG)/.stamp

distclean::
ifeq ("$(wildcard $(PKGDEV))","")
	-@$(RM) -rf $(PKGNAME)-*
else
	@unset CLEAN_DIRS
	-@$(MAKE) -C $(PKG) $@ $(REDIRECT)
	-@$(RM) $(PKG)/.stamp
endif

install::
	@echo "  INSTALL $(PKG)"
	+@$(MAKE) $(PKGENV) -C $(PKG) $(PKGINSTALL) $(REDIRECT)
