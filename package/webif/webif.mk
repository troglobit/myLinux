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

define WEBIF_INSTALL_TARGET_CMDS
        $(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) \
                DESTDIR="$(TARGET_DIR)" install
	$(INSTALL) $(BR2_EXTERNAL_MYLINUX_PATH)/package/webif/httpd.conf \
		$(TARGET_DIR)/etc/httpd.conf
endef

define WEBIF_INSTALL_INIT_SYSV
	$(INSTALL) -m 755 -D $(BR2_EXTERNAL_MYLINUX_PATH)/package/webif/S50webif \
		$(TARGET_DIR)/etc/init.d/S50webif
endef

define WEBIF_INSTALL_FINIT_SVC
	$(INSTALL) -D -m 0644 $(BR2_EXTERNAL_MYLINUX_PATH)/package/webif/webif.svc \
		$(TARGET_DIR)/etc/finit.d/available/webif.conf
	$(INSTALL) -d -m 0755 $(FINIT_D)/enabled
	ln -sf ../available/webif.conf $(FINIT_D)/enabled/webif.conf
endef

WEBIF_POST_INSTALL_TARGET_HOOKS += WEBIF_INSTALL_FINIT_SVC

$(eval $(generic-package))
