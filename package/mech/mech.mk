################################################################################
#
# mech
#
################################################################################

MECH_VERSION = 1.0
MECH_LICENSE = Apache-2.0
MECH_SITE_METHOD = local
MECH_SITE = $(BR2_EXTERNAL_MYLINUX_PATH)/src/mech
MECH_DEPENDENCIES = augeas clixon
MECH_AUTORECONF = YES

define MECH_INSTALL_EXTRA
	cp $(MECH_PKGDIR)/clixon.conf   $(FINIT_D)/available/
	ln -sf ../available/clixon.conf $(FINIT_D)/enabled/clixon.conf
	cp $(MECH_PKGDIR)/tmpfiles.conf $(TARGET_DIR)/etc/tmpfiles.d/mech.conf
	mkdir -p $(TARGET_DIR)/libexec
	cp $(MECH_PKGDIR)/prep-db       $(TARGET_DIR)/libexec/
	cp $(MECH_PKGDIR)/clean-etc     $(TARGET_DIR)/libexec/
endef
MECH_TARGET_FINALIZE_HOOKS += MECH_INSTALL_EXTRA

define MECH_GEN_YANG_TREE
	if which pyang 2>/dev/null; then \
		pyang -f tree \
			-p $(TARGET_DIR)/usr/share/clixon \
			$$(find $(TARGET_DIR)/usr/share/clixon/ -name '*@*.yang') >$(BINARIES_DIR)/mech-yang.txt; \
		pyang -f jstree \
			-p $(TARGET_DIR)/usr/share/clixon \
			$$(find $(TARGET_DIR)/usr/share/clixon/ -name '*@*.yang') >$(BINARIES_DIR)/mech-yang.html; \
	fi
endef
MECH_POST_INSTALL_TARGET_HOOKS += MECH_GEN_YANG_TREE

# The web interface builds its forms from the models, as JSON.  The tool
# that writes it reads them the way clixon does, features included.
ifeq ($(BR2_PACKAGE_WEBIF),y)
MECH_DEPENDENCIES += host-libyang

define MECH_BUILD_YANG2JSON
	$(HOSTCC) $(HOST_CFLAGS) -o $(@D)/yang2json $(@D)/tools/yang2json.c \
		$(HOST_LDFLAGS) -lyang
endef
MECH_POST_BUILD_HOOKS += MECH_BUILD_YANG2JSON

define MECH_GEN_SCHEMA
	mkdir -p $(TARGET_DIR)/var/www
	$(@D)/yang2json -p $(TARGET_DIR)/usr/share/clixon \
		$$(sed -n 's|.*<CLICON_FEATURE>\(.*\)</CLICON_FEATURE>.*|-F \1|p' $(@D)/clixon.xml.in) \
		mylinux >$(TARGET_DIR)/var/www/schema.json
	gzip -9kf $(TARGET_DIR)/var/www/schema.json
endef
MECH_POST_INSTALL_TARGET_HOOKS += MECH_GEN_SCHEMA
endif

$(eval $(autotools-package))
