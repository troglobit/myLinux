
CC               = $(CROSS_COMPILE)gcc
CFLAGS           =
CPPFLAGS         = -I$(STAGING)/include
BUSYBOX_VERSION  = 1.21.0
MINISNMP_VERSION = 1.2b
DROPBEAR_VERSION = 2013.58
BOARD            = EOMA68
PROGRAMS         = busybox

export CC CFLAGS CPPFLAGS

all:
	@for dir in $(PROGRAMS); do  \
		$(MAKE) -C $$dir $@; \
	done

install:
	@for dir in $(PROGRAMS); do  \
		$(MAKE) -C $$dir $@; \
	done

clean: 
	@for dir in $(PROGRAMS); do  \
		$(MAKE) -C $$dir $@; \
	done

distclean:
	@for dir in $(PROGRAMS); do  \
		$(MAKE) -C $$dir $@; \
	done

