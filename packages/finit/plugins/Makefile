PLUGINS   = mtd.so
CPPFLAGS += -I$(shell pwd)/../$(PKG) -I$(STAGING)/include
LDFLAGS  += -fPIC -shared

mtd.so: mtd.c
	@$(CC) $(CPPFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

clean:
	-@$(RM) $(PLUGINS)

distclean:
	-@$(RM) *.so *.o *~

install:
	@mkdir -p $(STAGING)/lib/finit/plugins
	@for plug in $(PLUGINS); do					\
		$(STRIP) -o $$STAGING/lib/finit/plugins/$$plug $$plug;	\
	done

