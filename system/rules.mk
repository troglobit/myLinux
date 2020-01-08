.PHONY: all install clean distclean
THIS := $(notdir $(shell pwd))

all:       $(addsuffix -all,       $(dir_y))
clean:     $(addsuffix -clean,     $(dir_y))
distclean: $(addsuffix -distclean, $(dir_all))
install:   $(addsuffix -install,   $(dir_y))

$(addsuffix -all, $(dir_y)):
	@echo "  BUILD   $(THIS)/$(patsubst %-all,%,$@)"
	+@$(MAKE) -C $(patsubst %-all,%,$@)

$(addsuffix -install, $(dir_y)):
	@echo "  INSTALL $(THIS)/$(patsubst %-install,%,$@)"
	+@$(MAKE) -C $(patsubst %-install,%,$@) install

$(addsuffix -clean, $(dir_y)):
	@echo "  CLEAN   $(THIS)/$(patsubst %-clean,%,$@)"
	+@$(MAKE) -C $(patsubst %-clean,%,$@) clean

$(addsuffix -distclean, $(dir_all)):
	@echo "  PROPER  $(THIS)/$(patsubst %-distclean,%,$@)"
	+@$(MAKE) -C $(patsubst %-distclean,%,$@) distclean
