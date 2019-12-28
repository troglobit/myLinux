# all/install/clean/distclean rules that iterate over $(dir_y)
THIS := $(notdir $(shell pwd))

all:
	@for dir in $(dir_y); do					\
		echo "  BUILD   $(THIS)/$$dir";				\
		$(MAKE) -C $$dir $@ || exit 1;				\
	done

install:
	@for dir in $(dir_y); do					\
		echo "  INSTALL $(THIS)/$$dir";				\
		$(MAKE) -C $$dir $@ || exit 1;				\
	done

clean: 
	-@for dir in $(dir_y); do					\
		echo "  CLEAN   $(THIS)/$$dir";				\
		$(MAKE) -C $$dir $@;					\
	done

distclean:
	-@for dir in $(dir_all); do					\
		echo "  REMOVE  $(THIS)/$$dir";				\
		$(MAKE) -C $$dir $@;					\
	done
