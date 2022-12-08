export BR2_EXTERNAL := $(CURDIR)
export PATH         := $(CURDIR)/utils:$(PATH)

ARCH   ?= $(shell uname -m)
O      ?= $(CURDIR)/output

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

.PHONY: all run debug
