export BR2_EXTERNAL := $(CURDIR):$(CURDIR)/netbox
export PATH         := $(CURDIR)/bin:$(PATH)

$(info $(BR2_EXTERNAL))
ARCH ?= $(shell uname -m)
O    ?= $(CURDIR)/output

config := $(O)/.config
bmake   = $(MAKE) -C netbox/buildroot O=$(O) $1


all: $(config) netbox/buildroot/Makefile
	@+$(call bmake,$@)

$(config):
	@+$(call bmake,list-defconfigs)
	@echo "ERROR: No configuration selected."
	@echo "Please choose a configuration from the list above by running"
	@echo "'make <board>_defconfig' before building an image."
	@exit 1

%: netbox/buildroot/Makefile
	@+$(call bmake,$@)

netbox/buildroot/Makefile:
	@git submodule update --init

run:
	@echo "Starting Qemu  ::  Ctrl-a x -- exit | Ctrl-a c -- toggle console/monitor"
	@qemu -f $(O)/images/qemu.cfg

.PHONY: all defconfig run
