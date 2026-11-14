################################################################################
#
# merecat
#
################################################################################

MERECAT_VERSION = 3.00-rc1
MERECAT_SITE = https://github.com/troglobit/merecat/releases/download/v$(MERECAT_VERSION)
MERECAT_SOURCE = merecat-$(MERECAT_VERSION).tar.gz
MERECAT_LICENSE = BSD-2-Clause
MERECAT_LICENSE_FILES = LICENSE
MERECAT_DEPENDENCIES = host-pkgconf libconfuse openssl zlib

# Supervised by Finit, invoked as 'merecat', so no systemd unit and no
# httpd/in.httpd compat symlinks (BusyBox httpd is disabled anyway).
MERECAT_CONF_OPTS = --without-systemd --without-symlinks

# Merecat's own start page, manual and sample CGIs have no place in
# /var/www on the device.  Of its web content only the icons are
# installed, below; error pages and directory listings use them.
MERECAT_INSTALL_TARGET_OPTS = DESTDIR=$(TARGET_DIR) install-exec

MERECAT_D = $(TARGET_DIR)/etc/finit.d

# Override the stock /etc/merecat.conf with ours, and install the TLS
# cert generator and the Finit service (enabled).
define MERECAT_INSTALL_EXTRA
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)/www/icons DESTDIR=$(TARGET_DIR) install
	$(INSTALL) -D -m 0644 $(MERECAT_PKGDIR)/merecat.conf \
		$(TARGET_DIR)/etc/merecat.conf
	$(INSTALL) -D -m 0755 $(MERECAT_PKGDIR)/gen-https-cert \
		$(TARGET_DIR)/usr/libexec/gen-https-cert
	$(INSTALL) -D -m 0644 $(MERECAT_PKGDIR)/merecat.svc \
		$(MERECAT_D)/available/merecat.conf
	$(INSTALL) -d -m 0755 $(MERECAT_D)/enabled
	ln -sf ../available/merecat.conf $(MERECAT_D)/enabled/merecat.conf
endef
MERECAT_POST_INSTALL_TARGET_HOOKS += MERECAT_INSTALL_EXTRA

$(eval $(autotools-package))
