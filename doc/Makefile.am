EXTRA_DIST = source

if ENABLE_DOCS

DOC_V_GEN = $(DOC_V_GEN_$(V))
DOC_V_GEN_ = $(DOC_V_GEN_$(AM_DEFAULT_VERBOSITY))
DOC_V_GEN_0 = @echo "  GEN   " $@;

.PHONY: docs

sphinx-doc: @PACKAGE@.tag

@PACKAGE@.tag: $(pkginclude_HEADERS) source/
	@rm -rf build
	$(DOC_V_GEN) $(SPHINXBUILD) @srcdir@/source/ build/
	@touch @PACKAGE@.tag

all-local: sphinx-doc

mostlyclean-local :
	rm -f @PACKAGE@.tag
	rm -rf build/

# distribute generated documentation
dist-hook: sphinx-doc
	cp -r build/ $(distdir)/

install-data-local: sphinx-doc
	test -d build/ && $(MKDIR_P) $(DESTDIR)$(docdir) && cp -r build $(DESTDIR)$(docdir)/manual

uninstall-local:
	rm -rf $(DESTDIR)$(docdir)/manual

endif ENABLE_DOCS
