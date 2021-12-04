export BR2_EXTERNAL := $(CURDIR)
export PATH         := $(CURDIR)/utils:$(PATH)
export BOARD         = `cat output/.config | grep BR2_DEFCONFIG | sed 's/.*\/\(.*\)_defconfig"/\1/'`

$(info $(BR2_EXTERNAL))
ARCH ?= $(shell uname -m)
O    ?= $(CURDIR)/output

config := $(O)/.config
bmake   = $(MAKE) -C buildroot O=$(O) $1


all: $(config) buildroot/Makefile
	@+$(call bmake,$@)

$(config):
	@+$(call bmake,list-defconfigs)
	@echo "ERROR: No configuration selected."
	@echo "Please choose a configuration from the list above by running"
	@echo "'make <board>_defconfig' before building an image."
	@exit 1

%: buildroot/Makefile
	@+$(call bmake,$@)

buildroot/Makefile:
	@git submodule update --init --recursive

$(O)/images/qemu.cfg: $(config)
	@cp $(CURDIR)/board/$(BOARD)/qemu.cfg $@

run: $(O)/images/qemu.cfg
	@echo "Starting Qemu  ::  Ctrl-a x -- exit | Ctrl-a c -- toggle console/monitor"
	@qemu -f $(O)/images/qemu.cfg

.PHONY: all run
