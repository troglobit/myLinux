################################################################################
#
# sntpd
#
################################################################################

SNTPD_VERSION = 3.1
SNTPD_SOURCE = sntpd-$(SNTPD_VERSION).tar.gz
SNTPD_SITE = https://github.com/troglobit/sntpd/releases/download/v$(SNTPD_VERSION)
SNTPD_LICENSE = GPL-2.0+
SNTPD_LICENSE_FILES = COPYING
SNTPD_INSTALL_STAGING = YES

define SNTPD_INSTALL_FINIT_SVC
	$(INSTALL) -D -m 0644 $(BR2_EXTERNAL_MYLINUX_PATH)/package/sntpd/sntpd.svc \
		$(FINIT_D)/available/sntpd.conf
endef

SNTPD_POST_INSTALL_TARGET_HOOKS += SNTPD_INSTALL_FINIT_SVC

$(eval $(autotools-package))
