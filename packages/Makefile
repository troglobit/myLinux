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
-include $(ROOTDIR)/.config

exclude                   = Makefile Kconfig $(wildcard *~)
dir_y                     =
dir_$(CONFIG_BUSYBOX)    += busybox
dir_$(CONFIG_DROPBEAR)   += dropbear
dir_$(CONFIG_FINIT)      += finit
dir_$(CONFIG_INADYN)     += inadyn
dir_$(CONFIG_IPROUTE2)   += iproute2
dir_$(CONFIG_IPTABLES)   += iptables
dir_$(CONFIG_LLDPD)      += lldpd
dir_$(CONFIG_LXC)        += lxc
dir_$(CONFIG_MERECAT)    += merecat
dir_$(CONFIG_MDNSD)      += mdnsd
dir_$(CONFIG_MG)         += mg
dir_$(CONFIG_MINI_SNMPD) += mini-snmpd
dir_$(CONFIG_MROUTED)    += mrouted
dir_$(CONFIG_NEMESIS)    += nemesis
dir_$(CONFIG_NETCALC)    += netcalc
dir_$(CONFIG_NETSNIFF)   += netsniff
dir_$(CONFIG_NFTABLES)   += nftables
dir_$(CONFIG_SNTPD)      += sntpd
dir_$(CONFIG_OMPING)     += omping
dir_$(CONFIG_MCJOIN)     += mcjoin
dir_$(CONFIG_PIMD)       += pimd
dir_$(CONFIG_REDIR)      += redir
dir_$(CONFIG_SMCROUTE)   += smcroute
dir_$(CONFIG_SOCAT)      += socat
dir_$(CONFIG_SSDPD)      += ssdp-responder
dir_$(CONFIG_STRACE)     += strace
dir_$(CONFIG_SYSKLOGD)   += sysklogd
dir_$(CONFIG_TETRIS)     += tetris
dir_$(CONFIG_TCPDUMP)    += tcpdump
dir_$(CONFIG_UFTPD)      += uftpd
dir_$(CONFIG_UREDIR)     += uredir
dir_$(CONFIG_WATCHDOGD)  += watchdogd
dir_n                     =
dir_                      =
dir_all                   = $(filter-out $(exclude), $(notdir $(wildcard *)))

include $(ROOTDIR)/system/rules.mk
