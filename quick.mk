$(addsuffix -build,$(TARGETS)):
	@echo "  BUILD   $(patsubst %-build,%,$@)"
	+@$(MAKE) -C $(patsubst %-build,%,$@) V=1 all

$(addsuffix -install,$(TARGETS)):
	@echo "  INSTALL $(patsubst %-install,%,$@)"
	+@$(MAKE) -C $(patsubst %-install,%,$@) V=1 install

$(addsuffix -chksum,$(TARGETS)):
	@echo "  CHKSUM  $(patsubst %-chksum,%,$@)"
	+@$(MAKE) -C $(patsubst %-chksum,%,$@) V=1 chksum

$(addsuffix -clean,$(TARGETS)):
	@echo "  CLEAN   $(patsubst %-clean,%,$@)"
	-+@$(MAKE) -C $(patsubst %-clean,%,$@) V=1 clean

$(addsuffix -distclean,$(TARGETS)):
	@echo "  PURGE   $(patsubst %-distclean,%,$@)"
	-+@$(MAKE) -C $(patsubst %-distclean,%,$@) V=1 distclean

# $(addsuffix -dev,$(TARGETS)):
# 	@echo "  DEV     $(patsubst %-dev,%,$@)"
# 	+@$(MAKE) -C $(patsubst %-dev,%,$@) V=1 dev

$(addsuffix -menuconfig,$(TARGETS)):
	@echo "  CONFIG  $(patsubst %-menuconfig,%,$@)"
	@$(MAKE) -C $(patsubst %-menuconfig,%,$@) menuconfig

$(addsuffix -oldconfig,$(TARGETS)):
	@echo "  CONFIG  $(patsubst %-oldconfig,%,$@)"
	@$(MAKE) -C $(patsubst %-oldconfig,%,$@) V=1 oldconfig

$(addsuffix -saveconfig,$(TARGETS)):
	@echo "  SAVING  $(patsubst %-saveconfig,%,$@)"
	@$(MAKE) -C $(patsubst %-saveconfig,%,$@) saveconfig
