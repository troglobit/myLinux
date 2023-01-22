################################################################################
#
# mdnsd
#
################################################################################

MDNSD_VERSION = 0.12
MDNSD_SITE = https://github.com/troglobit/mdnsd/releases/download/v$(MDNSD_VERSION)
MDNSD_LICENSE = BSD-3-Clause
MDNSD_LICENSE_FILES = LICENSE
MDNSD_INSTALL_STAGING = YES

ifeq ($(BR2_PACKAGE_MDNSD_MQUERY),y)
MDNSD_CONF_OPTS += --with-mquery
else
MDNSD_CONF_OPTS += --without-mquery
endif

ifeq ($(BR2_PACKAGE_SYSTEMD),y)
MDNSD_DEPENDENCIES += systemd
MDNSD_CONF_OPTS += --with-systemd
else
MDNSD_CONF_OPTS += --without-systemd
endif

ifeq ($(BR2_PACKAGE_MDNSD_FTP_SERVICE),y)
define MDNSD_INSTALL_FTP_SERVICE
	$(INSTALL) -D -m 0644 $(BR2_EXTERNAL_MYLINUX_PATH)/package/mdnsd/ftp.service \
		$(TARGET_DIR)/etc/mdns.d/
endef
MDNSD_POST_INSTALL_TARGET_HOOKS += MDNSD_INSTALL_FTP_SERVICE
endif

ifeq ($(BR2_PACKAGE_MDNSD_HTTP_SERVICE),y)
define MDNSD_INSTALL_HTTP_SERVICE
	$(INSTALL) -D -m 0644 $(BR2_EXTERNAL_MYLINUX_PATH)/package/mdnsd/http.service \
		$(TARGET_DIR)/etc/mdns.d/
endef
MDNSD_POST_INSTALL_TARGET_HOOKS += MDNSD_INSTALL_HTTP_SERVICE
endif

ifeq ($(BR2_PACKAGE_MDNSD_IPP_SERVICE),y)
define MDNSD_INSTALL_IPP_SERVICE
	$(INSTALL) -D -m 0644 $(BR2_EXTERNAL_MYLINUX_PATH)/package/mdnsd/ipp.service \
		$(TARGET_DIR)/etc/mdns.d/
endef
MDNSD_POST_INSTALL_TARGET_HOOKS += MDNSD_INSTALL_IPP_SERVICE
endif

ifeq ($(BR2_PACKAGE_MDNSD_PRINTER_SERVICE),y)
define MDNSD_INSTALL_PRINTER_SERVICE
	$(INSTALL) -D -m 0644 $(BR2_EXTERNAL_MYLINUX_PATH)/package/mdnsd/printer.service \
		$(TARGET_DIR)/etc/mdns.d/
endef
MDNSD_POST_INSTALL_TARGET_HOOKS += MDNSD_INSTALL_PRINTER_SERVICE
endif

ifeq ($(BR2_PACKAGE_MDNSD_SSH_SERVICE),y)
define MDNSD_INSTALL_SSH_SERVICE
	$(INSTALL) -D -m 0644 $(BR2_EXTERNAL_MYLINUX_PATH)/package/mdnsd/ssh.service \
		$(TARGET_DIR)/etc/mdns.d/
endef
MDNSD_POST_INSTALL_TARGET_HOOKS += MDNSD_INSTALL_SSH_SERVICE
endif

define MDNSD_INSTALL_INIT_SYSV
	$(INSTALL) -m 755 -D package/mdnsd/S50mdnsd $(TARGET_DIR)/etc/init.d/
endef

define MDNSD_INSTALL_INIT_SYSTEMD
	$(INSTALL) -D -m 644 $(@D)/mdnsd.service \
		$(TARGET_DIR)/usr/lib/systemd/system/mdnsd.service
endef

define MDNSD_INSTALL_FINIT_SVC
	$(INSTALL) -D -m 0644 $(BR2_EXTERNAL_MYLINUX_PATH)/package/mdnsd/mdnsd.svc \
		$(FINIT_D)/available/mdnsd.conf
	$(INSTALL) -d -m 0755 $(FINIT_D)/enabled
	ln -sf ../available/mdnsd.conf $(FINIT_D)/enabled/mdnsd.conf
endef

MDNSD_POST_INSTALL_TARGET_HOOKS += MDNSD_INSTALL_FINIT_SVC

$(eval $(autotools-package))
