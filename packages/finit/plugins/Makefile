PLUGINS   = mtd.so
CFLAGS    = -W -Wall -Wextra -Wno-unused-parameter
CPPFLAGS += -I$(shell pwd)/../$(PKG) -I$(STAGING)/include
LDFLAGS  += -fPIC -shared

mtd.so: mtd.c
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

clean:
	-@$(RM) $(PLUGINS)

distclean:
	-@$(RM) *.so *.o *~

install:
	@mkdir -p $(STAGING)/lib/finit/plugins
	@for plug in $(PLUGINS); do					\
		$(STRIP) -o $$STAGING/lib/finit/plugins/$$plug $$plug;	\
	done

