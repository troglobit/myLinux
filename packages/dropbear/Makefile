# Glue for cross compiling Dropbear SSH
PKGVER     := 2017.75
PKG        := dropbear-$(PKGVER)
PKGSUFFIX  := tar.bz2
PKGURL      = http://matt.ucc.asn.au/dropbear/releases/$(PKGTAR)
PKGENV      = PROGRAMS="dropbear dropbearkey dbclient scp" MULTI=1 SCPPROGRESS=1 DESTDIR=$(STAGING)
PKGCFGENV  := export ac_cv_member_struct_utmp_ut_host=yes;
PKGCFGENV  += export ac_cv_member_struct_utmp_ut_pid=yes;
PKGCFGENV  += export ac_cv_member_struct_utmp_ut_type=yes;
PKGCFG      = --host=$(CROSS_TARGET) --prefix= --sysconfdir=/mnt
PKGCFG     += --disable-wtmp --disable-shadow --disable-lastlog

include $(ROOTDIR)/system/pkg.mk

install::
	@mkdir -p $(STAGING)/etc/dropbear
