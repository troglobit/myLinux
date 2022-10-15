################################################################################
#
# finit-plugins
#
################################################################################

FINIT_PLUGINS_VERSION = 1.0
FINIT_PLUGINS_SITE = $(call github,troglobit,finit-plugins,$(FINIT_PLUGINS_VERSION))
FINIT_PLUGINS_LICENSE = MIT
FINIT_PLUGINS_LICENSE_FILES = LICENSE
FINIT_PLUGINS_DEPENDENCIES = host-pkgconf libite finit
FINIT_PLUGINS_CONF_OPTS = --prefix=/usr --disable-silent-rules

# Only when building from GIT hash
FINIT_PLUGINS_VERSION = f93132f564dbec1a2285bc8fa782a33906e9cb6d
FINIT_PLUGINS_AUTORECONF = YES
FINIT_PLUGINS_DEPENDENCIES += host-automake host-autoconf host-libtool

ifeq ($(BR2_PACKAGE_FINIT_PLUGINS_CHRONY),y)
FINIT_PLUGINS_CONF_OPTS += --enable-chrony-plugin
else
FINIT_PLUGINS_CONF_OPTS += --disable-chrony-plugin
endif

ifeq ($(BR2_PACKAGE_FINIT_PLUGINS_QMI_PROXY),y)
FINIT_PLUGINS_CONF_OPTS += --enable-qmi-proxy-plugin
else
FINIT_PLUGINS_CONF_OPTS += --disable-qmi-proxy-plugin
endif

$(eval $(autotools-package))
