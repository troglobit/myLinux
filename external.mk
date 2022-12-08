include $(sort $(wildcard $(BR2_EXTERNAL_MYLINUX_PATH)/package/*/*.mk))

.PHONY: local.mk
local.mk:
	@echo "Installing local override for certain packages ..."
	@(cd $O && ln -s $(BR2_EXTERNAL_MYLINUX_PATH)/local.mk .)

.PHONY: run
run:
	@if [ ! -f $(O)/images/qemu.sh ]; then \
		echo ">>> Qemu not supported yet for this target."; \
		exit 1; \
	fi
	@(cd $(O)/images && ./qemu.sh)

.PHONY: debug
debug:
	@[ -f $(O)/staging/.gdbinit ]    || cp $(CURDIR)/.gdbinit $(O)/staging/.gdbinit
	@[ -f $(O)/staging/.gdbinit.py ] || cp $(CURDIR)/.gdbinit.py $(O)/staging/.gdbinit.py
	@(cd $(O)/staging/ && gdb-multiarch)

