################################################################################
#
# nemesis
#
################################################################################

NEMESIS_VERSION = 1.8
NEMESIS_SITE    = https://github.com/libnet/nemesis/releases/download/v$(NEMESIS_VERSION)
NEMESIS_LICENSE = BSD-3-Clause
NEMESIS_LICENSE_FILES = LICENSE
NEMESIS_DEPENDENCIES = libnet

$(eval $(autotools-package))
