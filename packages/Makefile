# Free/Libre Open-Source Software Packages
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

CC               = $(CROSS_COMPILE)gcc
CFLAGS           =
CPPFLAGS         = -I$(STAGING)/include

export CC CFLAGS CPPFLAGS

exclude          = Makefile
dir_y            = busybox mini-snmpd
dir_n            =
dir_             =
dir_all          = $(filter-out $(exclude), $(notdir $(wildcard *)))


all:
	@for dir in $(dir_y); do					\
		echo "  BUILD   packages/$$dir";			\
		/bin/echo -ne "\033]0;$(PWD) packages/$$dir\007";	\
		$(MAKE) -C $$dir $@;					\
	done

install:
	@for dir in $(dir_y); do					\
		echo "  INSTALL packages/$$dir";			\
		/bin/echo -ne "\033]0;$(PWD) packages/$$dir\007";	\
		$(MAKE) -C $$dir $@;					\
	done

clean: 
	-@for dir in $(dir_y); do					\
		echo "  CLEAN   packages/$$dir";			\
		/bin/echo -ne "\033]0;$(PWD) packages/$$dir\007";	\
		$(MAKE) -C $$dir $@;					\
	done

distclean:
	-@for dir in $(dir_y); do					\
		echo "  REMOVE  packages/$$dir";			\
		/bin/echo -ne "\033]0;$(PWD) packages/$$dir\007";	\
		$(MAKE) -C $$dir $@;					\
	done

$(addsuffix -build,$(dir_all)):
	@echo "  BUILD   packages/$(patsubst %-distclean,%,$@)"
	+@$(MAKE) -C $(patsubst %-build,%,$@)

$(addsuffix -romfs,$(dir_all)):
	@echo "  INSTALL packages/$(patsubst %-romfs,%,$@)"
	+@$(MAKE) -C $(patsubst %-romfs,%,$@) romfs $(REDIRECT)

$(addsuffix -clean,$(dir_all)):
	@echo "  CLEAN   packages/$(patsubst %-clean,%,$@)"
	-+@$(MAKE) -C $(patsubst %-clean,%,$@) clean

$(addsuffix -distclean,$(dir_all)):
	@echo "  REMOVE  packages/$(patsubst %-distclean,%,$@)"
	-+@$(MAKE) -C $(patsubst %-distclean,%,$@) distclean

$(addsuffix -menuconfig,$(dir_all)):
	@echo "  CONFIG  packages/$(patsubst %-menuconfig,%,$@)"
	@$(MAKE) -C $(patsubst %-menuconfig,%,$@) menuconfig

$(addsuffix -saveconfig,$(dir_all)):
	@echo "  SAVING  packages/$(patsubst %-saveconfig,%,$@)"
	@$(MAKE) -C $(patsubst %-saveconfig,%,$@) saveconfig

