# Assumes PKG, VER and URL being set already. Alternatively set PKGNAME.
# If the package is not tar.gz packed, set PKGSUFFIX to the proper suffix.

PKGNAME		?= $(PKG:-$(VER)=)
PKGSUFFIX       ?= tar.gz
TARBALL		?= $(PKG).$(PKGSUFFIX)
TMPFILE          = $(ROOTDIR)/tmp/$(TARBALL)
ARCHIVE		?= $(DOWNLOADS)/$(PKGNAME)/$(TARBALL)
PRIMARY_URL	?= $(BIN_REPO)/$(PKGNAME)/$(TARBALL)

$(ARCHIVE):
	@echo "  FETCH   $(PKG) ..." 1>&2
	@mkdir -p $(dir $@)
	@mkdir -p $(ROOTDIR)/tmp
	@curl -L -o $(TMPFILE) $(PRIMARY_URL) || curl -L -o $(TMPFILE) $(URL)
	@mv $(TMPFILE) $@

download: $(ARCHIVE)

checksums/$(PKG).sha1: $(ARCHIVE)
	@echo "Generating SHA1 checksum for $(PKG) ..."
	@mkdir -p checksums
	(cd $(dir $(ARCHIVE)); sha1sum $(PKG)*) > $@
	git add $@

gensum: checksums/$(PKG).sha1
