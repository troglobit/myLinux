# Glue for cross compiling Mini SNMP Daemon
VER            ?= 1.2b
NAME            = mini_snmpd
PKG		= $(NAME)-$(VER)
PKGSUFFIX	= tar.gz
PATCHES 	= patches/*.patch
URL             = http://members.aon.at/linuxfreak/linux/$(PKG).$(PKGSUFFIX)
CFLAGS         := $(filter-out -W -Wall, $(CFLAGS))
CFLAGS         := $(filter-out -Werror, $(CFLAGS))
CFLAGS         := $(filter-out -Wcast-align, $(CFLAGS))
CFLAGS         := $(filter-out -Wformat, $(CFLAGS))

all: build

include $(ROOTDIR)/package.mk

$(PKG)/Makefile: $(ARCHIVE) $(PATCHES) patches/series
	-@$(RM) -r $(PKG)
	@tar xfj $(ARCHIVE) && mv $(NAME) $(PKG)
	@echo "  PATCH   $(PKG)/"
	@(cd $(PKG); ln -sf ../patches .)
	@(cd $(PKG); quilt push -qa)
	@touch $@

unpack: $(PKG)/Makefile

$(PKG)/.stamp: $(PKG)/Makefile
	+@$(MAKE) -C $(PKG) OFLAGS="-O2 -D__IPV4__ -static" CC=$(CC)
	@touch $@

build: $(PKG)/.stamp

install:
	@$(CROSS_COMPILE)strip -o $(STAGING)/sbin/$(NAME) $(PKG)/$(NAME)

clean:
	-@$(MAKE) -C $(PKG) clean
	-@$(RM) $(PKG)/.stamp

distclean:
	-@$(RM) busybox-*

