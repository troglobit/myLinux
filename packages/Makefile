# Free/Libre Open-Source Software Packages
#
# Copyright (c) 2014-2016  Joachim Nilsson <troglobit@gmail.com>
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

-include $(ROOTDIR)/.config

exclude                   = Makefile
dir_y                     =
dir_$(CONFIG_BUSYBOX)    += busybox
dir_$(CONFIG_DROPBEAR)   += dropbear
dir_$(CONFIG_FINIT)      += finit
dir_$(CONFIG_INADYN)     += inadyn
dir_$(CONFIG_LLDPD)      += lldpd
dir_$(CONFIG_MG)         += mg
dir_$(CONFIG_MINI_SNMPD) += mini-snmpd
dir_$(CONFIG_MROUTED)    += mrouted
dir_$(CONFIG_NETCALC)    += netcalc
dir_$(CONFIG_OMPING)     += omping
dir_$(CONFIG_MCJOIN)     += mcjoin
dir_$(CONFIG_PIMD)       += pimd
dir_$(CONFIG_SMCROUTE)   += smcroute
dir_$(CONFIG_STRACE)     += strace
dir_$(CONFIG_TETRIS)     += tetris
dir_$(CONFIG_UFTPD)      += uftpd
dir_$(CONFIG_WATCHDOGD)  += watchdogd
dir_n                     =
dir_                      =
dir_all                   = $(filter-out $(exclude), $(notdir $(wildcard *)))

include ../rules.mk

