# Glue for cross compiling SMCRoute

PKGVER     := 2.0.0
PKG        := smcroute-$(PKGVER)
PKGSUFFIX  := tar.xz
PKGURL      = $(TROGLOHUB)/$(PKGNAME)/releases/download/$(PKGVER)/$(PKGTAR)
PKGPATCHES :=
PKGCFG     := --host=$(CROSS_TARGET) --prefix=
PKGCFGENV  := ac_cv_func_setpgrp_void=yes ac_cv_func_malloc_0_nonnull=yes ac_cv_func_memcmp_working=yes
PKGINSTALL := install-exec

include $(ROOTDIR)/pkg.mk
