################################################################################
#
# mdnsd
#
################################################################################
MDNSD_VERSION = 0.10
MDNSD_SOURCE = mdnsd-$(MDNSD_VERSION).tar.xz
MDNSD_SITE = https://github.com/troglobit/mdnsd/releases/download/v$(MDNSD_VERSION)
MDNSD_LICENSE = BSD-3-Clause

define MDNSD_INSTALL_SAMPLE_CONFIG
	$(INSTALL) -D -m 0644 $(BR2_EXTERNAL_MYLINUX)/package/mdmsd/ssh.service \
		$(STAGING)/etc/mdns.d/
endef

define MDNSD_INSTALL_FINIT_SVC
	$(INSTALL) -D -m 0644 $(BR2_EXTERNAL_MYLINUX)/package/mdnsd/mdnsd.svc \
		$(STAGING)/etc/finit.d/
endef

MDNSD_POST_INSTALL_TARGET_HOOKS += MDNSD_INSTALL_SAMPLE_CONFIG
MDNSD_POST_INSTALL_TARGET_HOOKS += MDNSD_INSTALL_FINIT_SVC
