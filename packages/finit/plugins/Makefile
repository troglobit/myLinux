PLUGINS   = mtd.so
LDFLAGS += -fPIC -shared -I$(shell pwd)/../$(PKG)

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

