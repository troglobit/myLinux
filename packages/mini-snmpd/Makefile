# Glue for cross compiling Mini SNMP Daemon
include $(ROOTDIR)/core.mk

PKGVER     := 1.3
PKG        := mini-snmpd-$(PKGVER)
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGCFG     := --prefix= --host=$(CROSS_TARGET)
PKGINSTALL := install-exec

include $(ROOTDIR)/pkg.mk
