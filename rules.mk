# all/install/clean/distclean rules that iterate over $(dir_y)
THIS := $(notdir $(shell pwd))

all:
	@for dir in $(dir_y); do					\
		echo "  BUILD   $(THIS)/$$dir" | tee -a $(BUILDLOG);	\
		/bin/echo -ne "\033]0;$(PWD) $(THIS)/$$dir\007";	\
		$(MAKE) -C $$dir $@ $(REDIRECT) || exit 1;		\
	done

install:
	@for dir in $(dir_y); do					\
		echo "  INSTALL $(THIS)/$$dir" | tee -a $(BUILDLOG);	\
		/bin/echo -ne "\033]0;$(PWD) $(THIS)/$$dir\007";	\
		$(MAKE) -C $$dir $@ $(REDIRECT) || exit 1;		\
	done

clean: 
	-@for dir in $(dir_y); do					\
		echo "  CLEAN   $(THIS)/$$dir" | tee -a $(BUILDLOG);	\
		/bin/echo -ne "\033]0;$(PWD) $(THIS)/$$dir\007";	\
		$(MAKE) -C $$dir $@ $(REDIRECT);			\
	done

distclean:
	-@for dir in $(dir_y); do					\
		echo "  REMOVE  $(THIS)/$$dir" | tee -a $(BUILDLOG);	\
		/bin/echo -ne "\033]0;$(PWD) $(THIS)/$$dir\007";	\
		$(MAKE) -C $$dir $@ $(REDIRECT);			\
	done
