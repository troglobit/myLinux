#############################################################
#
# webif
#
#############################################################

WEBIF_VERSION = 1.0
WEBIF_SITE_METHOD = local
WEBIF_SITE = $(BR2_EXTERNAL_MYLINUX_PATH)/src/webif
WEBIF_LICENSE = GPL-2.0

# Static files only; Merecat is the httpd that serves /var/www.
define WEBIF_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/var/www
	cp -r $(@D)/www/. $(TARGET_DIR)/var/www/
endef

$(eval $(generic-package))
