# all/install/clean/distclean rules that iterate over $(dir_y)
THIS := $(notdir $(shell pwd))

all:
	@for dir in $(dir_y); do				\
		echo "  BUILD   $(THIS)/$$dir";			\
		/bin/echo -ne "\033]0;$(PWD) $(THIS)/$$dir\007";\
		$(MAKE) -C $$dir $@;				\
	done

install:
	@for dir in $(dir_y); do				\
		echo "  INSTALL $(THIS)/$$dir";			\
		/bin/echo -ne "\033]0;$(PWD) $(THIS)/$$dir\007";\
		$(MAKE) -C $$dir $@;				\
	done

clean: 
	-@for dir in $(dir_y); do				\
		echo "  CLEAN   $(THIS)/$$dir";			\
		/bin/echo -ne "\033]0;$(PWD) $(THIS)/$$dir\007";\
		$(MAKE) -C $$dir $@;				\
	done

distclean:
	-@for dir in $(dir_y); do				\
		echo "  REMOVE  $(THIS)/$$dir";			\
		/bin/echo -ne "\033]0;$(PWD) $(THIS)/$$dir\007";\
		$(MAKE) -C $$dir $@;				\
	done
