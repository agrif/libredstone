# =======================
# tools using libredstone
# =======================

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/src
LDADD = $(top_builddir)/src/libredstone.la

bin_PROGRAMS = redstone

redstone_SOURCES = main.c options.h optparse.c optparse.h
redstone_SOURCES += actions.h action-extract.c
redstone_SOURCES += formats.h formats.c format-prettyprint.c format-raw.c
