EXTRA_DIST = source

if DX_COND_doc

DX_V_GEN = $(DX_V_GEN_$(V))
DX_V_GEN_ = $(DX_V_GEN_$(AM_DEFAULT_VERBOSITY))
DX_V_GEN_0 = @echo "  GEN   " $@;

.PHONY: docs

sphinx-doc: @PACKAGE@.tag

@PACKAGE@.tag: $(pkginclude_HEADERS) source/
	@rm -rf build
	$(DX_V_GEN) sphinx-build @srcdir@/source/ build/
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

endif DX_COND_doc
