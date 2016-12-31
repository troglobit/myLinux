$(addsuffix -build,$(TARGETS)):
	@echo "  BUILD   $(patsubst %-build,%,$@)"           | tee -a $(BUILDLOG)
	+@$(MAKE) -C $(patsubst %-build,%,$@) all            $(REDIRECT)

$(addsuffix -install,$(TARGETS)):
	@echo "  INSTALL $(patsubst %-install,%,$@)"         | tee -a $(BUILDLOG)
	+@$(MAKE) -C $(patsubst %-install,%,$@) install      $(REDIRECT)

$(addsuffix -chksum,$(TARGETS)):
	@echo "  CHKSUM  $(patsubst %-chksum,%,$@)"          | tee -a $(BUILDLOG)
	+@$(MAKE) -C $(patsubst %-chksum,%,$@) chksum        $(REDIRECT)

$(addsuffix -clean,$(TARGETS)):
	@echo "  CLEAN   $(patsubst %-clean,%,$@)"           | tee -a $(BUILDLOG)
	-+@$(MAKE) -C $(patsubst %-clean,%,$@) clean         $(REDIRECT)

$(addsuffix -distclean,$(TARGETS)):
	@echo "  REMOVE  $(patsubst %-distclean,%,$@)"       | tee -a $(BUILDLOG)
	-+@$(MAKE) -C $(patsubst %-distclean,%,$@) distclean $(REDIRECT)

$(addsuffix -dev,$(TARGETS)):
	@echo "  DEV     $(patsubst %-dev,%,$@)"             | tee -a $(BUILDLOG)
	+@$(MAKE) -C $(patsubst %-dev,%,$@) dev              $(REDIRECT)

$(addsuffix -menuconfig,$(TARGETS)):
	@echo "  CONFIG  $(patsubst %-menuconfig,%,$@)"      | tee -a $(BUILDLOG)
	@$(MAKE) -C $(patsubst %-menuconfig,%,$@) menuconfig

$(addsuffix -oldconfig,$(TARGETS)):
	@echo "  CONFIG  $(patsubst %-oldconfig,%,$@)"      | tee -a $(BUILDLOG)
	@$(MAKE) -C $(patsubst %-oldconfig,%,$@) oldconfig

$(addsuffix -saveconfig,$(TARGETS)):
	@echo "  SAVING  $(patsubst %-saveconfig,%,$@)"      | tee -a $(BUILDLOG)
	@$(MAKE) -C $(patsubst %-saveconfig,%,$@) saveconfig $(REDIRECT)
