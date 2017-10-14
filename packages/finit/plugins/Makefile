PLUGINS   = mtd.so
ifdef CONFIG_FINIT_BOOT_ANIMATION
PLUGINS  += logo.so
endif
CFLAGS    = -W -Wall -Wextra -Wno-unused-parameter
CPPFLAGS += -I$(shell pwd)/../$(PKG) -I$(STAGING)/include
LDFLAGS  += -fPIC -shared

all: $(PLUGINS)

%.so: %.c
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

mtd.so: mtd.c

logo.so: logo.c

clean:
	-@$(RM) $(PLUGINS)

distclean:
	-@$(RM) *.so *.o *~

install:
	@mkdir -p $(STAGING)/lib/finit/plugins
	@for plug in $(PLUGINS); do					\
		$(STRIP) -o $$STAGING/lib/finit/plugins/$$plug $$plug;	\
	done

