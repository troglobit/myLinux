include $(top_srcdir)/common.am

# Debug helpers linked into every backend plugin.
noinst_LTLIBRARIES = common.la
common_la_LDFLAGS  = $(cli_ldflags)
common_la_LIBADD   = $(cli_libadd)
common_la_SOURCES  = common.c

# Core: front-end CLI and its backend.
clispec_DATA += core-cli.cli

cli_LTLIBRARIES += core-cli.la
core_cli_la_LDFLAGS = $(cli_ldflags)
core_cli_la_LIBADD  = $(cli_libadd)
core_cli_la_SOURCES = core-cli.c cli-show.c

backend_LTLIBRARIES += core-backend.la
core_backend_la_CFLAGS  = $(backend_cflags)
core_backend_la_LDFLAGS = $(backend_ldflags)
core_backend_la_LIBADD  = $(backend_libadd)
core_backend_la_SOURCES = core-backend.c

# Per-model backend plugins.  Helper files use a model prefix, e.g. sys-*.c.
backend_LTLIBRARIES += system.la
system_la_CFLAGS  = $(backend_cflags)
system_la_LDFLAGS = $(backend_ldflags)
system_la_LIBADD  = $(backend_libadd)
system_la_SOURCES = system.c

backend_LTLIBRARIES += interfaces.la
interfaces_la_CFLAGS  = $(backend_cflags)
interfaces_la_LDFLAGS = $(backend_ldflags)
interfaces_la_LIBADD  = $(backend_libadd)
interfaces_la_SOURCES = interfaces.c

install-exec-hook:
	find $(DESTDIR)$(clixonlibdir) -name '*.la' -delete
