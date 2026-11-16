#############################################################
#
# webif
#
#############################################################

WEBIF_VERSION = 1.0
WEBIF_SITE_METHOD = local
WEBIF_SITE = $(BR2_EXTERNAL_MYLINUX_PATH)/src/webif
WEBIF_LICENSE = GPL-2.0

# Static files only; Merecat is the httpd that serves /var/www.  It has
# the length of a file compressed ahead of time, and with that can keep
# the connection for the next one.
define WEBIF_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/var/www
	cp -r $(@D)/www/. $(TARGET_DIR)/var/www/
	cd $(TARGET_DIR)/var/www && gzip -9kf index.html webif.css webif.js
endef

$(eval $(generic-package))
