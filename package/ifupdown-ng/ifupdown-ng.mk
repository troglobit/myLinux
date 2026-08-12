################################################################################
#
# ifupdown-ng
#
################################################################################

IFUPDOWN_NG_VERSION = 0.13.0
IFUPDOWN_NG_SITE = $(call github,ifupdown-ng,ifupdown-ng,ifupdown-ng-$(IFUPDOWN_NG_VERSION))
IFUPDOWN_NG_LICENSE = ISC
IFUPDOWN_NG_LICENSE_FILES = COPYING

# Plain Makefile, no configure.  The default linux layout uses shell
# executors that call iproute2, so neither libmnl (netlink) nor libbsd is
# needed -- uClibc-ng provides strlcpy()/strlcat().  Override CFLAGS to
# drop the upstream -Werror, but leave CPPFLAGS alone so the Makefile's
# -DINTERFACES_FILE and friends survive.
define IFUPDOWN_NG_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) \
		CC="$(TARGET_CC)" AR="$(TARGET_AR)" \
		CFLAGS="$(TARGET_CFLAGS)" LDFLAGS="$(TARGET_LDFLAGS)" \
		LIBBSD_CFLAGS= LIBBSD_LIBS= LIBMNL_CFLAGS= LIBMNL_LIBS=
endef

# ifupdown-ng and BusyBox both provide /sbin/if{up,down,query}; drop any
# stale symlinks first so the install's `ln -s` does not fail on them.
define IFUPDOWN_NG_INSTALL_TARGET_CMDS
	$(RM) $(addprefix $(TARGET_DIR)/sbin/,ifup ifdown ifquery ifctrstat ifparse)
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) DESTDIR=$(TARGET_DIR) install
endef

$(eval $(generic-package))
