# Find a suitable toolchain at https://github.com/myrootfs/crosstool-ng/releases
TOOLCHAIN         := crosstool-ng-1.23.0-319-gaca85cb

OSNAME            := myLinux
OSRELEASE_ID      := chaos
OSRELEASE          = Chaos Devel `date --iso-8601`
OSVERSION_ID      := 1.0-rc1
OSVERSION         := $(OSVERSION_ID), $(OSRELEASE)
OSID              := "mylinux"
OSPRETTY_NAME     := $(OSNAME) $(OSVERSION_ID)
OSHOME_URL        := https://myrootfs.github.io
SUPPORT_URL       := https://github.com/myrootfs/myLinux
BUG_REPORT_URL    := $(SUPPORT_URL)/issues

STAGING            = $(ROOTDIR)/staging
ROMFS              = $(ROOTDIR)/romfs
IMAGEDIR           = $(ROOTDIR)/images
HOSTDIR           ?= $(shell xdg-helper DOCUMENTS)/myLinux
PKG_CONFIG_LIBDIR := $(STAGING)/lib/pkgconfig

# System should be configured by this point
-include $(ROOTDIR)/.config

# Check to be sure, CONFIG_DOT_CONFIG only set if include succeeeded
ifeq ($(CONFIG_DOT_CONFIG),y)
qstrip             = $(strip $(subst ",,$(1)))
# "

ARCH              := $(call qstrip, $(CONFIG_ARCH))
MACH              := $(call qstrip, $(CONFIG_MACH))

IMAGE_NAME        := $(call qstrip, $(CONFIG_IMAGE_NAME))
KERNEL_IMAGE       = $(call qstrip, $(CONFIG_LINUX_IMAGE))
KERNEL_VERSION    := $(call qstrip, $(CONFIG_LINUX_VERSION))
ifdef KERNEL_RC
KERNEL_VERSION     = $(KERNEL_VERSION).0$(KERNEL_RC)
endif
KERNEL_MODULES     = $(wildcard $(ROMFS)/lib/modules/$(KERNEL_VERSION)*)

# Map Qemu archs to Linux kernel archs
KERNEL_ARCH       := $(shell echo $(ARCH) | sed	\
			-e 's/ppc64/powerpc64/'	\
			-e 's/ppc/powerpc/'	\
			-e 's/aarch64/arm64/'   \
			-e 's/x86_64/x86/')

# Include QEMU_ settings for this arch.
include $(ROOTDIR)/arch/$(ARCH)/config.mk

KERNEL            ?= $(IMAGEDIR)/$(KERNEL_IMAGE)
KERNEL_APPEND     ?= $(call qstrip, $(CONFIG_LINUX_CMDLINE))
ROOTFS            ?= $(IMAGEDIR)/$(IMAGE_NAME)
DISK              ?= $(HOSTDIR)/mnt-$(ARCH).jffs2

CROSS_COMPILE     := $(call qstrip, $(CONFIG_TOOLCHAIN_PREFIX))
CROSS_TARGET       = $(CROSS_COMPILE:-=)
CC                 = $(CROSS_COMPILE)gcc
CFLAGS             = -O2
#CPPFLAGS           = -U_FORTIFY_SOURCE -D_DEFAULT_SOURCE -D_GNU_SOURCE
CPPFLAGS           = -I$(STAGING)/include -I$(STAGING)/usr/include
LDLIBS             =
LDFLAGS            = -L$(STAGING)/lib -L$(STAGING)/usr/lib
STRIP              = $(CROSS_COMPILE)strip
endif

export OSNAME OSRELEASE_ID OSRELEASE OSVERSION_ID OSVERSION
export OSID OSPRETTY_NAME OSHOME_URL
export SUPPORT_URL BUG_REPORT_URL

export ARCH MACH CROSS_COMPILE CROSS_TARGET TOOLCHAIN
export CC CFLAGS CPPFLAGS LDLIBS LDFLAGS STRIP
export KERNEL ROOTFS
export STAGING ROMFS IMAGEDIR HOSTDIR PKG_CONFIG_LIBDIR
export QEMU_ARCH QEMU_MACH QEMU_NIC QEMU_SCSI QEMU_EXTRA QEMU_DTB
export KERNEL KERNEL_APPEND ROOTFS DISK
export KERNEL_ARCH KERNEL_VERSION KERNEL_MODULES
