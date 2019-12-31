# Glue for cross compiling Mini SNMP Daemon
PKGVER     := 1.4
PKG        := mini-snmpd-$(PKGVER)
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGCFG      = --host=$(CROSS_TARGET) --prefix=
PKGINSTALL := install-exec

include $(ROOTDIR)/pkg.mk
