# Glue for cross compiling Ncurses
PKGVER     := 5.9
PKG        := ncurses-$(PKGVER)
PKGPATCHES :=
PKGINSTALL := install.libs install.includes install.data
PKGSUFFIX  := tar.gz
PKGURL      = http://ftp.gnu.org/gnu/$(PKGNAME)/$(PKGTAR)
PKGCFGENV  := CPPFLAGS=-P
PKGCFG      = --host=$(CROSS_TARGET) --prefix= --program-prefix=
PKGCFG     += --disable-static --disable-rpath-hack --disable-big-core		\
	      --enable-termcap --enable-sigwinch  --enable-pc-files		\
	      --enable-hard-tabs --enable-xmc-glitch --enable-colorfgbg		\
	      --enable-overwrite \
	      --without-debug --without-ada --without-gpm --without-manpages	\
	      --without-profile --without-cxx-binding --without-progs		\
	      --without-tests --with-shared --with-terminfo --with-termlib=tinfo
#	      --with-ticlib --disable-database
#	      --with-termpath='${sysconfdir}/termcap:${datadir}/misc/termcap'
#	      --with-terminfo-dirs='${sysconfdir}/terminfo:${datadir}/terminfo' 

include $(ROOTDIR)/pkg.mk

install::
	@cp  -f libtermcap.so $(STAGING)/lib/
	@ln -sf libtinfo.a    $(STAGING)/lib/libtermcap.a
