# Glue for cross compiling the Linux kernel
#
# Copyright (c) 2014-2016  Joachim Nilsson <troglobit@gmail.com>
#
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted, provided that the
# above copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
# IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
.PHONY: all defconfig oldconfig saveconfig headers_install modules_install clean distclean

include ../core.mk

PKGVER         ?= $(KERNEL_VERSION)
PKGBASEVER     := $(shell echo $(PKGVER) | sed 's/\([0-9]*\.[0-9]*\).*/\1/')
PKG            := linux-$(PKGVER)
PKGNAME	       := kernel
PKGSUFFIX      := tar.xz
PKGTARGETS     := kernel headers_install modules modules_install
PKGPATCHES     := patches/$(PKGBASEVER)/*.patch patches/$(PKGBASEVER)/series
ifdef CONFIG_LINUX_RC
RC_PATH         = testing/
endif
PKGURL          = http://cdn.kernel.org/pub/linux/kernel/v4.x/$(RC_PATH)$(PKGTAR)
CONFIG          = $(PRODDIR)/linux.config-$(PKGBASEVER)
ZIMAGE          = $(PKG)/arch/$(KERNEL_ARCH)/boot/$(KERNEL_IMG)
unexport KBUILD_KCONFIG
include $(ROOTDIR)/pkg.mk

$(PKG)/.config::
	@if [ ! -s $(PKG)/.config ]; then							\
		echo "  INSTALL $@ from $(CONFIG)" | tee -a $(BUILDLOG);			\
		if [ -f $(CONFIG) ]; then							\
			cp $(CONFIG) $@;							\
			$(MAKE) -C $(PKG) ARCH=$(KERNEL_ARCH) oldconfig;			\
		else										\
			$(MAKE) -C $(PKG) ARCH=$(KERNEL_ARCH) $(MACH)_defconfig $(REDIRECT);	\
		fi;										\
		echo "  PREPARE $(PKG)" | tee -a $(BUILDLOG);					\
		$(MAKE) -C $(PKG) ARCH=$(KERNEL_ARCH) prepare scripts $(REDIRECT);		\
	else											\
		echo "  NO-OP   $(PKG)/.config already installed."				\
					| tee -a $(BUILDLOG); 					\
	fi

$(ZIMAGE): $(PKG)/.config
	@echo "  BUILD   $(PKG)" | tee -a $(BUILDLOG)
	+@$(MAKE) -C $(PKG) ARCH=$(KERNEL_ARCH) $(KERNEL_IMG) $(REDIRECT)

defconfig:
	@$(MAKE) -C $(PKG) ARCH=$(KERNEL_ARCH) $(MACH)_defconfig $(REDIRECT)

oldconfig:: $(PKG)/.config
	@$(MAKE) -C $(PKG) ARCH=$(KERNEL_ARCH) oldconfig

saveconfig:
	@echo -n "  SAVING  "
	@cp -v $(PKG)/.config $(CONFIG)

menuconfig: $(PKG)/.config
	@echo "  CONFIG  $(PKG)" | tee -a $(BUILDLOG)
	@$(MAKE) -C $(PKG) ARCH=$(KERNEL_ARCH) menuconfig

dtbinst:
	@if [ "$(ARCH)" = "arm" ]; then \
		echo "  DTBINST kernel/$(PKG)" | tee -a $(BUILDLOG); \
		INSTALL_DTBS_PATH=$(IMAGEDIR) $(MAKE) -C $(PKG) ARCH=$(KERNEL_ARCH) dtbs dtbs_install $(REDIRECT); \
	fi

kernel: $(ZIMAGE)
	@echo "  INSTALL kernel/$(PKG)" | tee -a $(BUILDLOG)
	@cp -v $(ZIMAGE) $(IMAGEDIR) $(REDIRECT)

modules: kernel
	@echo "  MODULES $(PKG)" | tee -a $(BUILDLOG)
	+@$(MAKE) -C $(PKG) ARCH=$(KERNEL_ARCH) modules $(REDIRECT)

headers_install: kernel
	@echo "  HEADERS kernel/$(PKG)" | tee -a $(BUILDLOG)
	+$(MAKE) -C $(PKG) $@ ARCH=$(KERNEL_ARCH) INSTALL_HDR_PATH=$(STAGING) $(REDIRECT)

#modules:
#	rm -rf modules
#	mkdir -p modules/lib/modules/$(KERNELRELEASE)
#	sh -ec "cd $(KERNEL_MODULES);find * -depth -print0 | cpio -0pdm $(PWD)/modules/lib/modules/$(KERNELRELEASE)"
#	find modules/lib/modules/$(KERNELRELEASE) -name '*.ko' | xargs $(CROSS_COMPILE)strip -g

modules_install: modules
	@echo "  MODINST kernel/$(PKG)" | tee -a $(BUILDLOG)
	@. $(PKG)/.config;						\
	if [ "$$CONFIG_MODULES" = "y" ]; then				\
		if [ ! -d $(STAGING)/lib/modules ]; then		\
			mkdir -p $(STAGING)/lib/modules;		\
		fi;							\
		$(MAKE) -C $(PKG) ARCH=$(KERNEL_ARCH)			\
				  INSTALL_MOD_PATH=$(STAGING)		\
				  INSTALL_MOD_STRIP=--strip-all		\
				  $@ $(REDIRECT);			\
	fi

distclean::
	-@$(RM) -rf linux-*
