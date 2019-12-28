# To be included by top-level Makefile
# Set ARCH=ppc when calling make to get a ppc *_defconfig
#ARCH           ?= arm
kcfg           := $(ROOTDIR)/kconfig
KBUILD_KCONFIG := $(ROOTDIR)/Kconfig

PHONY += oldconfig menuconfig config silentoldconfig
PHONY += allnoconfig allyesconfig allmodconfig alldefconfig randconfig
PHONY += listnewconfig oldnoconfig savedefconfig defconfig

ifdef KBUILD_KCONFIG
Kconfig := $(KBUILD_KCONFIG)
else
Kconfig := Kconfig
endif

menuconfig: $(kcfg)/mconf dep					## Update current config utilising a menu based program
	@$< $(Kconfig)

config: $(kcfg)/conf						## Update current config utilising a line-oriented program
	@$< --oldaskconfig $(Kconfig)

oldconfig: $(kcfg)/conf						## Update current config utilising a provided .config as base
	@$< --$@ $(Kconfig)

silentoldconfig: $(kcfg)/conf					## Same as oldconfig, but quietly, additionally update deps
	@mkdir -p include/generated
	@$< --$@ $(Kconfig)

allnoconfig allyesconfig allmodconfig alldefconfig randconfig: $(kcfg)/conf
	@$< --$@ $(Kconfig)

listnewconfig oldnoconfig: $(kcfg)/conf				## List new options
	@$< --$@ $(Kconfig)

savedefconfig: $(kcfg)/conf
	@$< --$@=defconfig $(Kconfig)

defconfig: $(kcfg)/conf						## Use TroglOS defconfig
ifeq ($(KBUILD_DEFCONFIG),)
	@$< --defconfig $(Kconfig)
else
	@echo "*** Default configuration is based on '$(KBUILD_DEFCONFIG)'"
	@$< --defconfig=arch/$(ARCH)/configs/$(KBUILD_DEFCONFIG) $(Kconfig)
endif

%_defconfig: $(kcfg)/conf
	@echo "  CONFIG  $(ARCH)/configs/$@ ..."
	@$< --defconfig=arch/$(ARCH)/configs/$@ $(Kconfig)

$(kcfg)/conf:
	@$(MAKE) -C $(kcfg) conf

$(kcfg)/mconf:
	@$(MAKE) -C $(kcfg) mconf

