################################################################################
#
# uftpd
#
################################################################################

UFTPD_VERSION = 2.15
UFTPD_SITE = https://github.com/troglobit/uftpd/releases/download/v$(UFTPD_VERSION)
UFTPD_LICENSE = ISC
UFTPD_LICENSE_FILES = LICENSE
UFTPD_INSTALL_STAGING = YES
UFTPD_DEPENDENCIES = host-pkgconf libite libuev

define UFTPD_INSTALL_FINIT_SVC
	$(INSTALL) -D -m 0644 $(BR2_EXTERNAL_MYLINUX_PATH)/package/uftpd/uftpd.def \
		$(TARGET_DIR)/etc/default/uftpd
	$(INSTALL) -D -m 0644 $(BR2_EXTERNAL_MYLINUX_PATH)/package/uftpd/uftpd.svc \
		$(TARGET_DIR)/etc/finit.d/available/uftpd.conf
endef

UFTPD_POST_INSTALL_TARGET_HOOKS += UFTPD_INSTALL_FINIT_SVC

$(eval $(autotools-package))
