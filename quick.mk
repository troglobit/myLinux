$(addsuffix -build,$(TARGETS)):
	@echo "  BUILD   $(patsubst %-build,%,$@)"                | tee -a $(BUILDLOG)
	+@$(MAKE) -C $(patsubst %-build,%,$@) V=1 all        2>&1 | teepee $(BUILDLOG)

$(addsuffix -install,$(TARGETS)):
	@echo "  INSTALL $(patsubst %-install,%,$@)"              | tee -a $(BUILDLOG)
	+@$(MAKE) -C $(patsubst %-install,%,$@) V=1 install  2>&1 | teepee $(BUILDLOG)

# $(addsuffix -chksum,$(TARGETS)):
# 	@echo "  CHKSUM  $(patsubst %-chksum,%,$@)"               | tee -a $(BUILDLOG)
# 	+@$(MAKE) -C $(patsubst %-chksum,%,$@) V=1 chksum    2>&1 | teepee $(BUILDLOG)

$(addsuffix -clean,$(TARGETS)):
	@echo "  CLEAN   $(patsubst %-clean,%,$@)"                | tee -a $(BUILDLOG)
	-+@$(MAKE) -C $(patsubst %-clean,%,$@) V=1 clean     2>&1 | teepee $(BUILDLOG)

$(addsuffix -distclean,$(TARGETS)):
	@echo "  PURGE   $(patsubst %-distclean,%,$@)"                | tee -a $(BUILDLOG)
	-+@$(MAKE) -C $(patsubst %-distclean,%,$@) V=1 distclean 2>&1 | teepee $(BUILDLOG)

# $(addsuffix -dev,$(TARGETS)):
# 	@echo "  DEV     $(patsubst %-dev,%,$@)"                  | tee -a $(BUILDLOG)
# 	+@$(MAKE) -C $(patsubst %-dev,%,$@) V=1 dev          2>&1 | teepee $(BUILDLOG)

$(addsuffix -menuconfig,$(TARGETS)):
	@echo "  CONFIG  $(patsubst %-menuconfig,%,$@)"           | tee -a $(BUILDLOG)
	@$(MAKE) -C $(patsubst %-menuconfig,%,$@) menuconfig

$(addsuffix -oldconfig,$(TARGETS)):
	@echo "  CONFIG  $(patsubst %-oldconfig,%,$@)"            | tee -a $(BUILDLOG)
	@$(MAKE) -C $(patsubst %-oldconfig,%,$@) V=1 oldconfig

$(addsuffix -saveconfig,$(TARGETS)):
	@echo "  SAVING  $(patsubst %-saveconfig,%,$@)"           | tee -a $(BUILDLOG)
	@$(MAKE) -C $(patsubst %-saveconfig,%,$@) saveconfig 2>&1 | teepee $(BUILDLOG)
