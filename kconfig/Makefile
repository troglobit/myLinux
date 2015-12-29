# ===========================================================================
# Kernel configuration targets
# These targets are used from top-level makefile
VPATH := $(kcfg)

# lxdialog stuff
check-lxdialog  := $(kcfg)/lxdialog/check-lxdialog.sh

# Use recursively expanded variables so we do not call gcc unless
# we really need to do so. (Do not call gcc as part of make mrproper)
HOST_CFLAGS += $(shell $(check-lxdialog) -ccflags) -DLOCALE

# ===========================================================================
# Shared Makefile for the various kconfig executables:
# conf:	  Used for defconfig, oldconfig and related targets
# mconf:  Used for the menuconfig target
#         Utilizes the lxdialog package
# object files used by all kconfig flavours

lxdialog := lxdialog/checklist.o lxdialog/util.o lxdialog/inputbox.o
lxdialog += lxdialog/textbox.o lxdialog/yesno.o lxdialog/menubox.o

conf-objs   := $(kcfg)/conf.o  $(kcfg)/zconf.tab.o
mconf-objs  := $(kcfg)/mconf.o $(kcfg)/zconf.tab.o $(addprefix $(kcfg)/, $(lxdialog))
OBJS        := $(conf-objs) $(mconf-objs) 
DEPS        := $(OBJS:.o=.d)

hostprogs-y := $(kcfg)/conf

ifeq ($(MAKECMDGOALS),menuconfig)
	hostprogs-y += $(kcfg)/mconf
endif

clean-files	:= lkc_defs.h qconf.moc .tmp_qtcheck .tmp_gtkcheck
clean-files	+= zconf.tab.c lex.zconf.c zconf.hash.c gconf.glade.h
clean-files     += mconf qconf gconf nconf conf
clean-files     += config.pot linux.pot

# Check that we have the required ncurses stuff installed for lxdialog (menuconfig)
PHONY += dochecklxdialog
$(addprefix ,$(lxdialog)): dochecklxdialog
dochecklxdialog:
	$(Q)$(check-lxdialog) -check $(HOSTCC) $(HOST_CFLAGS) $(HOSTLOADLIBES_mconf)

always := dochecklxdialog

# Add environment specific flags
HOST_CFLAGS += $(shell $(kcfg)/check.sh $(HOSTCC) $(HOSTCFLAGS))

# generated files seem to need this to find local include files
HOSTCFLAGS_lex.zconf.o	:= -I$(kcfg)
HOSTCFLAGS_zconf.tab.o	:= -I$(kcfg)

HOSTLOADLIBES_mconf   = $(shell $(check-lxdialog) -ldflags $(HOSTCC))

$(kcfg)/zconf.tab.o: $(kcfg)/lex.zconf.c $(kcfg)/zconf.hash.c

$(kcfg)/kconfig_load.o: lkc_defs.h

lkc_defs.h: $(kcfg)/lkc_proto.h
	$(Q)sed < $< > $@ 's/P(\([^,]*\),.*/#define \1 (\*\1_p)/'

###
# The following requires flex/bison/gperf
# By default we use the _shipped versions, uncomment the following line if
# you are modifying the flex/bison src.
# LKC_GENPARSER := 1

ifdef LKC_GENPARSER

$(kcfg)/zconf.tab.c: $(kcfg)/zconf.y
$(kcfg)/lex.zconf.c: $(kcfg)/zconf.l
$(kcfg)/zconf.hash.c: $(kcfg)/zconf.gperf

%.tab.c: %.y
	bison -l -b $* -p $(notdir $*) $<
	cp $@ $@_shipped

lex.%.c: %.l
	flex -L -P$(notdir $*) -o$@ $<
	cp $@ $@_shipped

%.hash.c: %.gperf
	gperf < $< > $@
	cp $@ $@_shipped
else
$(kcfg)/%:: $(kcfg)/%_shipped
	cat $< > $@
endif

%.o: %.c
	$(HOSTCC) $(HOST_CFLAGS) -c -o $@ $<

$(kcfg)/mconf: $(mconf-objs)
	$(HOSTCC) -o $@ $^ $(HOST_CFLAGS) $(HOSTLOADLIBES_mconf)

$(kcfg)/conf: $(conf-objs)
	$(HOSTCC) -o $@ $^ $(HOST_CFLAGS)

-include $(DEPS)
