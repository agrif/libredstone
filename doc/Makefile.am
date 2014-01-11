# Copyright (C) 2004 Oren Ben-Kiki
# This file is distributed under the same terms as the Automake macro files.
# and has been modified for use in libredstone

# Generate automatic documentation using Doxygen. Goals and variables values
# are controlled by the various DX_COND_??? conditionals set by autoconf.
#
# The provided goals are:
# doxygen-doc: Generate all doxygen documentation.
# doxygen-run: Run doxygen, which will generate some of the documentation
#              (HTML, CHM, CHI, MAN, RTF, XML) but will not do the post
#              processing required for the rest of it (PS, PDF, and some MAN).
# doxygen-man: Rename some doxygen generated man pages.
# doxygen-ps: Generate doxygen PostScript documentation.
# doxygen-pdf: Generate doxygen PDF documentation.
#
# Note that by default these are not integrated into the automake goals. If
# doxygen is used to generate man pages, you can achieve this integration by
# setting man3_MANS to the list of man pages generated and then adding the
# dependency:
#
#   $(man3_MANS): doxygen-doc
#
# This will cause make to run doxygen and generate all the documentation.
#
# The following variable is intended for use in Makefile.am:
#
# DX_CLEANFILES = everything to clean.
#
# This is usually added to MOSTLYCLEANFILES.

## --------------------------------- ##
## Format-independent Doxygen rules. ##
## --------------------------------- ##

if DX_COND_doc

DX_V_GEN = $(DX_V_GEN_$(V))
DX_V_GEN_ = $(DX_V_GEN_$(AM_DEFAULT_VERBOSITY))
DX_V_GEN_0 = @echo "  GEN   " $@;

SPHINX_V_GEN = $(SPHINX_V_GEN_$(V))
SPHINX_V_GEN_ = $(SPHINX_V_GEN_$(AM_DEFAULT_VERBOSITY))
SPHINX_V_GEN_0 = @echo "  GEN   " $@;

DX_V_LATEX = $(DX_V_LATEX_$(V))
DX_V_LATEX_ = $(DX_V_LATEX_$(AM_DEFAULT_VERBOSITY))
DX_V_LATEX_0 = @echo "  LATEX " $@;

## ------------------------------- ##
## Rules specific for HTML output. ##
## ------------------------------- ##

if DX_COND_html

DX_CLEAN_HTML = @DX_DOCDIR@/html

endif DX_COND_html

## ------------------------------ ##
## Rules specific for CHM output. ##
## ------------------------------ ##

if DX_COND_chm

DX_CLEAN_CHM = @DX_DOCDIR@/chm

if DX_COND_chi

DX_CLEAN_CHI = @DX_DOCDIR@/@PACKAGE@.chi

endif DX_COND_chi

endif DX_COND_chm

## ------------------------------ ##
## Rules specific for MAN output. ##
## ------------------------------ ##

if DX_COND_man

DX_CLEAN_MAN = @DX_DOCDIR@/man

endif DX_COND_man

## ------------------------------ ##
## Rules specific for RTF output. ##
## ------------------------------ ##

if DX_COND_rtf

DX_CLEAN_RTF = @DX_DOCDIR@/rtf

endif DX_COND_rtf

## ------------------------------ ##
## Rules specific for XML output. ##
## ------------------------------ ##

if DX_COND_xml

DX_CLEAN_XML = @DX_DOCDIR@/xml

endif DX_COND_xml

## ----------------------------- ##
## Rules specific for PS output. ##
## ----------------------------- ##

if DX_COND_ps

DX_LATEX_ARGS = $(DX_LATEX) -interaction=batchmode

DX_CLEAN_PS = @DX_DOCDIR@/@PACKAGE@.ps

DX_PS_GOAL = doxygen-ps

doxygen-ps: @DX_DOCDIR@/@PACKAGE@.ps

@DX_DOCDIR@/@PACKAGE@.ps: @DX_DOCDIR@/@PACKAGE@.tag
	$(DX_V_LATEX) cd @DX_DOCDIR@/latex; \
	rm -f *.aux *.toc *.idx *.ind *.ilg *.log *.out; \
	$(DX_LATEX_ARGS) refman.tex 1>/dev/null; \
	$(DX_MAKEINDEX) refman.idx 1>/dev/null 2>&1; \
	$(DX_LATEX_ARGS) refman.tex 1>/dev/null; \
	countdown=5; \
	while $(DX_EGREP) 'Rerun (LaTeX|to get cross-references right)' \
	                  refman.log > /dev/null 2>&1 \
	   && test $$countdown -gt 0; do \
	    $(DX_LATEX_ARGS) refman.tex 1>/dev/null; \
	    countdown=`expr $$countdown - 1`; \
	done; \
	$(DX_DVIPS) -o ../@PACKAGE@.ps refman.dvi 1>/dev/null 2>&1

endif DX_COND_ps

## ------------------------------ ##
## Rules specific for PDF output. ##
## ------------------------------ ##

if DX_COND_pdf

DX_PDFLATEX_ARGS = $(DX_PDFLATEX) -interaction=batchmode

DX_CLEAN_PDF = @DX_DOCDIR@/@PACKAGE@.pdf

DX_PDF_GOAL = doxygen-pdf

doxygen-pdf: @DX_DOCDIR@/@PACKAGE@.pdf

@DX_DOCDIR@/@PACKAGE@.pdf: @DX_DOCDIR@/@PACKAGE@.tag
	$(DX_V_LATEX) cd @DX_DOCDIR@/latex; \
	rm -f *.aux *.toc *.idx *.ind *.ilg *.log *.out; \
	$(DX_PDFLATEX_ARGS) refman.tex 1>/dev/null; \
	$(DX_MAKEINDEX) refman.idx 1>/dev/null 2>&1; \
	$(DX_PDFLATEX_ARGS) refman.tex 1>/dev/null; \
	countdown=5; \
	while $(DX_EGREP) 'Rerun (LaTeX|to get cross-references right)' \
	                  refman.log > /dev/null 2>&1 \
	   && test $$countdown -gt 0; do \
	    $(DX_PDFLATEX_ARGS) refman.tex 1>/dev/null; \
	    countdown=`expr $$countdown - 1`; \
	done; \
	mv refman.pdf ../@PACKAGE@.pdf

endif DX_COND_pdf

## ------------------------------------------------- ##
## Rules specific for LaTeX (shared for PS and PDF). ##
## ------------------------------------------------- ##

if DX_COND_latex

DX_CLEAN_LATEX = @DX_DOCDIR@/latex

endif DX_COND_latex

.PHONY: doxygen-run doxygen-doc $(DX_PS_GOAL) $(DX_PDF_GOAL)

.INTERMEDIATE: doxygen-run $(DX_PS_GOAL) $(DX_PDF_GOAL)

doxygen-run: @DX_DOCDIR@/@PACKAGE@.tag

doxygen-doc: doxygen-run $(DX_PS_GOAL) $(DX_PDF_GOAL)

@DX_DOCDIR@/@PACKAGE@.tag: $(DX_CONFIG) $(pkginclude_HEADERS)
	@rm -rf @DX_DOCDIR@
	$(DX_V_GEN) $(DX_ENV) $(DX_DOXYGEN) $(DX_CONFIG)
	@rm -rf build
	$(SPHINX_V_GEN) sphinx-build source/ build/
	@touch @DX_DOCDIR@/@PACKAGE@.tag

DX_CLEANFILES = \
    @DX_DOCDIR@/@PACKAGE@.tag \
    $(DX_CLEAN_HTML) \
    $(DX_CLEAN_CHM) \
    $(DX_CLEAN_CHI) \
    $(DX_CLEAN_MAN) \
    $(DX_CLEAN_RTF) \
    $(DX_CLEAN_XML) \
    $(DX_CLEAN_PS) \
    $(DX_CLEAN_PDF) \
    $(DX_CLEAN_LATEX)

all-local: doxygen-doc
mostlyclean-local :
	rm -rf $(DX_CLEANFILES)
	rm -rf @DX_DOCDIR@

# distribute generated documentation
dist-hook: doxygen-doc
	cp -r @DX_DOCDIR@ $(distdir)/

install-data-local: doxygen-doc
	test -d @DX_DOCDIR@ && $(MKDIR_P) $(DESTDIR)$(docdir) && cp -r @DX_DOCDIR@ $(DESTDIR)$(docdir)

uninstall-local:
	rm -rf $(DESTDIR)$(docdir)/@DX_DOCDIR@

endif DX_COND_doc
