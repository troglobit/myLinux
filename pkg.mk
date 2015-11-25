# Assumes PKG, VER and URL being set already. Alternatively set PKGNAME.
# If the package is not tar.gz packed, set PKGSUFFIX to the proper suffix.

FETCH           ?= wget -t3 -nc --no-dns-cache --no-iri -q -cO
PKGNAME         ?= $(PKG:-$(VER)=)
PKGSUFFIX       ?= tar.gz
TARBALL         ?= $(PKG).$(PKGSUFFIX)
TMPFILE          = $(ROOTDIR)/tmp/$(TARBALL)
ARCHIVE         ?= $(DOWNLOADS)/$(PKGNAME)/$(TARBALL)
MIRROR          ?= $(FTP_MIRROR)/$(PKGNAME)/$(TARBALL)

$(ARCHIVE):
	@mkdir -p $(dir $@)
	@mkdir -p $(ROOTDIR)/tmp
	@if [ x"$(FTP_MIRROR)" != x ]; then		\
		echo "  FETCH   $(MIRROR) ...";		\
		$(FETCH) $(TMPFILE) $(MIRROR);		\
	else						\
		echo "  FETCH   $(URL) ...";		\
		$(FETCH) $(TMPFILE) $(URL);		\
	fi
	@mv $(TMPFILE) $@
	@if [ $$? -ne 0 ]; then				\
		rm $(TMPFILE);				\
		exit 1;					\
	fi

download: $(ARCHIVE)

checksums/$(PKG).sha1: $(ARCHIVE)
	@echo "Generating SHA1 checksum for $(PKG) ..."
	@mkdir -p checksums
	(cd $(dir $(ARCHIVE)); sha1sum $(PKG)*) > $@
	git add $@

gensum: checksums/$(PKG).sha1
