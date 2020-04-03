# Glue for cross compiling mdnsd
PKGVER     := 0.9
PKG        := mdnsd-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = https://github.com/troglobit/$(PKGNAME)/releases/download/v$(PKGVER)/$(PKGTAR)
PKGPATCHES :=
PKGCFG      = --host=$(CROSS_TARGET) --prefix=
PKGINSTALL := install-exec

include $(ROOTDIR)/system/pkg.mk

install::
	@mkdir -p $(STAGING)/etc/mdns.d
	@cp ssh.service $(STAGING)/etc/mdns.d/
