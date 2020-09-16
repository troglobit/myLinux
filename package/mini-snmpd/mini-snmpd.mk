################################################################################
#
# minit-snmpd
#
################################################################################
MINI_SNMPD_VERSION = 1.4
MINI_SNMPD_SOURCE = mini-snmpd-$(MINI_SNMPD_VERSION).tar.gz
MINI_SNMPD_SITE = https://github.com/troglobit/mini-snmpd/releases/download/v$(MINI_SNMPD_VERSION)
MINI_SNMPD_LICENSE = GPL-2.0+

define MINI_SNMPD_INSTALL_FINIT_SVC
	$(INSTALL) -D -m 0644 $(BR2_EXTERNAL_MYLINUX)/package/mini-snmpd/mini-snmpd.svc \
		$(STAGING)/etc/finit.d/
endef

MINI_SNMPD_POST_INSTALL_TARGET_HOOKS += MINI_SNMPD_INSTALL_FINIT_SVC
