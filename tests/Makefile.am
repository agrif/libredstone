# =================
# libredstone tests
# =================

check_PROGRAMS = memory error endian
noinst_HEADERS = test-boilerplate.h
EXTRA_DIST = runtests.sh colors.sh
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/src -I$(srcdir)
LDADD = $(top_builddir)/src/libredstone.la

check-local :
	@echo
	@$(srcdir)/runtests.sh $(check_PROGRAMS)
