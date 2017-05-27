# Glue for cross compiling Open Multicast Ping
include $(ROOTDIR)/core.mk

PKGVER     := 0.0.5
PKG        := omping-$(PKGVER)
PKGPATCHES :=
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/$(PKGVER)/$(PKGTAR)
PKGENV     := DESTDIR=$(STAGING) PREFIX=
PKGINSTALL := install-exec

include $(ROOTDIR)/pkg.mk
