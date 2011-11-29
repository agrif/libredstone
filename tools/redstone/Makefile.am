# =======================
# tools using libredstone
# =======================

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/src
LIBS =
LDADD = $(top_builddir)/src/libredstone.la $

bin_PROGRAMS = redstone

redstone_SOURCES = main.c options.h optparse.c optparse.h
redstone_SOURCES += actions.h action-extract.c

redstone_SOURCES += formats.h formats.c format-prettyprint.c format-raw.c

if ENABLE_LIBXML
INCLUDES += ${libxml2_CFLAGS}
LIBS += ${libxml2_LIBS}
redstone_SOURCES += format-xml.c
endif
