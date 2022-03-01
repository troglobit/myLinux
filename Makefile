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

run:
	@if [ ! -f $(O)/images/qemu.sh ]; then \
		echo ">>> Qemu not supported yet for this target."; \
		exit 1; \
	fi
	@echo "Starting Qemu  ::  Ctrl-a x -- exit | Ctrl-a c -- toggle console/monitor"
	@(cd $(O)/images && ./qemu.sh)

debug:
	@[ -f $(O)/staging/.gdbinit ]    || cp $(CURDIR)/.gdbinit $(O)/staging/.gdbinit
	@[ -f $(O)/staging/.gdbinit.py ] || cp $(CURDIR)/.gdbinit.py $(O)/staging/.gdbinit.py
	@(cd $(O)/staging/ && gdb-multiarch)

.PHONY: all run debug
