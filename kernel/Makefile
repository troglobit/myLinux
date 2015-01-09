# Glue for cross compiling the Linux kernel
VER            ?= $(KERNEL_VERSION)
BASE_VER       := $(shell echo $(VER) | sed 's/\([0-9]*\.[0-9]*\)\.[0-9]*/\1/')
PKG             = linux-$(VER)
PKGNAME	        = kernel
PKGSUFFIX       = tar.xz
TARBALL         = $(PKG).$(PKGSUFFIX)
URL             = ftp://ftp.kernel.org/pub/linux/kernel/v3.x/$(TARBALL)
CONFIG          = linux-$(BASE_VER).config
ZIMAGE          = $(PKG)/arch/$(ARCH)/boot/zImage

all: kernel headers_install modules modules_install

include ../package.mk

$(PKG)/Makefile: $(ARCHIVE)
	@echo "  UNPACK  $(PKG)"
	@tar Jxf $(ARCHIVE)
	@touch $@

unpack: $(PKG)/Makefile

$(PKG)/.prepared: $(PKG)/Makefile
	@$(MAKE) -C $(PKG) prepare scripts
	@touch $@

$(PKG)/.config: $(PKG)/Makefile
	@echo "  INSTALL $@ from $(CONFIG)"
	@if [ -f $(CONFIG) ]; then			\
		cp $(CONFIG) $@;			\
	else						\
		$(MAKE) -C $(PKG) versatile_defconfig;	\
	fi

$(ZIMAGE): $(PKG)/.config $(PKG)/.prepared
	@echo "  BUILD   $(PKG)";
	+@$(MAKE) -C $(PKG) zImage

defconfig:

oldconfig: $(PKG)/.config
	@$(MAKE) -C $(PKG) oldconfig 2>/dev/null

saveconfig:
	@cp -v $(PKG)/.config $(CONFIG)

menuconfig:
	@$(MAKE) -C $(PKG) menuconfig

kernel: $(ZIMAGE)
	@cp $(ZIMAGE) $(IMAGEDIR)

modules: kernel
	@echo "  MODULES $(PKG)";
	+@$(MAKE) -C $(PKG) modules

headers_install:
	+$(MAKE) -C $(PKG) headers_install ARCH=$(ARCH) INSTALL_HDR_PATH=$(STAGING)

modules_install:
	@. $(PKG)/.config;					\
	if [ "$$CONFIG_MODULES" = "y" ]; then			\
		if [ ! -d $(STAGING)/lib/modules ]; then	\
			mkdir -p $(STAGING)/lib/modules;	\
		fi;						\
		$(MAKE) -C $(PKG) INSTALL_MOD_PATH=$(STAGING)	\
				  INSTALL_MOD_STRIP=--strip-all	\
				  modules_install;		\
	fi

