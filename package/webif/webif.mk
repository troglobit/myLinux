#############################################################
#
# webif
#
#############################################################

WEBIF_VERSION = 0.2
WEBIF_SITE_METHOD = local
WEBIF_SITE = $(BR2_EXTERNAL_MYLINUX_PATH)/src/webif
WEBIF_LICENSE = GPL-2.0
WEBIF_INSTALL_STAGING = YES

define WEBIF_BUILD_CMDS
        $(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) \
                LDLIBS="$(TARGET_LDFLAGS)"
endef

# The web UI content only; Merecat is the httpd that serves /var/www.
define WEBIF_INSTALL_TARGET_CMDS
        $(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) \
                DESTDIR="$(TARGET_DIR)" install
endef

$(eval $(generic-package))
