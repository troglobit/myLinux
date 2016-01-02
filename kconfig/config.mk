# To be included by top-level Makefile
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

menuconfig: $(kcfg)/mconf
	@$< $(Kconfig)

config: $(kcfg)/conf
	@$< --oldaskconfig $(Kconfig)

oldconfig: $(kcfg)/conf
	@$< --$@ $(Kconfig)

silentoldconfig: $(kcfg)/conf
	@mkdir -p include/generated
	@$< --$@ $(Kconfig)

allnoconfig allyesconfig allmodconfig alldefconfig randconfig: $(kcfg)/conf
	@$< --$@ $(Kconfig)

listnewconfig oldnoconfig: $(kcfg)/conf
	@$< --$@ $(Kconfig)

savedefconfig: $(kcfg)/conf
	@$< --$@=defconfig $(Kconfig)

defconfig: $(kcfg)/conf
ifeq ($(KBUILD_DEFCONFIG),)
	@$< --defconfig $(Kconfig)
else
	@echo "*** Default configuration is based on '$(KBUILD_DEFCONFIG)'"
	@$< --defconfig=configs/$(KBUILD_DEFCONFIG) $(Kconfig)
endif

%_defconfig: $(kcfg)/conf
	@$< --defconfig=configs/$@ $(Kconfig)

# Help text used by make help
help:
	@echo  '  config	  - Update current config utilising a line-oriented program'
	@echo  '  menuconfig	  - Update current config utilising a menu based program'
	@echo  '  oldconfig	  - Update current config utilising a provided .config as base'
	@echo  '  localmodconfig  - Update current config disabling modules not loaded'
	@echo  '  silentoldconfig - Same as oldconfig, but quietly, additionally update deps'
	@echo  '  defconfig	  - New config with default from ARCH supplied defconfig'
	@echo  '  savedefconfig   - Save current config as ./defconfig (minimal config)'
	@echo  '  allnoconfig	  - New config where all options are answered with no'
	@echo  '  allyesconfig	  - New config where all options are accepted with yes'
	@echo  '  allmodconfig	  - New config selecting modules when possible'
	@echo  '  alldefconfig    - New config with all symbols set to default'
	@echo  '  randconfig	  - New config with random answer to all options'
	@echo  '  listnewconfig   - List new options'
	@echo  '  oldnoconfig     - Same as silentoldconfig but set new symbols to n (unset)'

$(kcfg)/conf:
	@$(MAKE) -C $(kcfg) conf

$(kcfg)/mconf:
	@$(MAKE) -C $(kcfg) mconf

