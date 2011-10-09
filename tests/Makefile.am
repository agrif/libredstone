# =================
# libredstone tests
# =================

check_PROGRAMS = memory error
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/src
LDADD = $(top_builddir)/src/libredstone.la

check-local :
	@echo
	@$(srcdir)/runtests.sh $(check_PROGRAMS)
