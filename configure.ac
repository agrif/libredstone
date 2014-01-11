dnl =============================
dnl Version Rules and AM/AC setup
dnl =============================

AC_PREREQ(2.63.2)

dnl user-facing version number, usual (major.minor.patch) scheme
m4_define(libredstone_major_version, 0)
m4_define(libredstone_minor_version, 1)
m4_define(libredstone_patch_version, 0)

dnl libtool library version, current:revision:age scheme
m4_define(libredstone_lt_current_version, 1)
m4_define(libredstone_lt_revision_version, 0)
m4_define(libredstone_lt_age_version, 0)

dnl user-facing copyright info, mostly used in docs
m4_define(libredstone_copyright, [2014 Aaron Griffith])

dnl Handy notes for version management!
dnl -----------------------------------
dnl
dnl Patch should be bumped for any release that does not change the interface.
dnl Minor should be bumped for most interface-changing releases.
dnl Major should be bumped any time there is a major restructuring.
dnl
dnl Note that minor changes should *try* to remain
dnl backwards-compatible, and that the 1.0.0 release may have the same
dnl structure as the 0.x.x releases. The '0' major version is special
dnl that way.
dnl
dnl For the libtool version:
dnl * If the code has changed at all, bump revision.
dnl * If any interfaces have changed, bump current ant set revision to 0.
dnl * If there are new interfaces, bump age.
dnl * If there are backwards-incompatible changes, set age to 0.
dnl
dnl With this in mind, a patch version will just bump revision. A
dnl minor version will *usually* bump current and age, and set
dnl revision to 0. A major version will bump current and set revision
dnl and age to 0.

m4_define(libredstone_version, [libredstone_major_version.libredstone_minor_version.libredstone_patch_version])
m4_define(libredstone_lt_version, [libredstone_lt_current_version:libredstone_lt_revision_version:libredstone_lt_age_version])

AC_INIT([libredstone], [libredstone_version])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/redstone.h])
AC_CONFIG_MACRO_DIR([m4])

AC_DEFINE(LIBREDSTONE_VERSION, "libredstone_version", [libredstone version])
AC_SUBST(LIBREDSTONE_VERSION, libredstone_version)
AC_DEFINE(LIBREDSTONE_LT_VERSION, "libredstone_lt_version", [libredstone libtool version])
AC_SUBST(LIBREDSTONE_LT_VERSION, libredstone_lt_version)
AC_DEFINE(LIBREDSTONE_COPYRIGHT, "libredstone_copyright", [libredstone copyright])
AC_SUBST(LIBREDSTONE_COPYRIGHT, libredstone_copyright)

AM_INIT_AUTOMAKE([1.11 -Wall foreign nostdinc dist-bzip2 no-dist-gzip])
AM_MAINTAINER_MODE([enable])

AM_SILENT_RULES([yes])

dnl =====================
dnl Basic Compiler Checks
dnl =====================

AC_PROG_CC
if test "$GCC" == "yes"; then
   CFLAGS="-Wall $CFLAGS"
fi
AC_PROG_CC_C_O
AC_PROG_CC_C99
if test "$av_cv_prog_cc_c99" == "no"; then
   AC_MSG_ERROR([libredstone requires a C99-compliant C compiler])
fi

AC_PROG_INSTALL
AC_PROG_MKDIR_P

LT_PREREQ([2.2.6])
LT_INIT

AC_HEADER_ASSERT
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_CHECK_FUNC(mmap, [], AC_MSG_ERROR([libredstone needs a mmap implementation to work]))

dnl =================
dnl Dependency Checks
dnl =================

AC_CHECK_HEADER(zlib.h, [], AC_MSG_ERROR([libredstone needs zlib installed to function]))
AC_CHECK_LIB(z, inflateEnd, [LIBS="-lz $LIBS"], AC_MSG_ERROR([libredstone needs zlib installed to function]))

dnl ======
dnl Python
dnl ======

PYTHON_REQUIRED=2.6.0

AC_ARG_ENABLE(python,
			  AS_HELP_STRING([--enable-python], [Enable Python support]),
			  [enable_python=$enableval],
			  [enable_python=auto])

if test "$enable_python" = "no"; then
	found_python="no (disabled, use --enable-python to enable)"
else
	AM_PATH_PYTHON($PYTHON_REQUIRED,
				   [found_python=yes],
				   [found_python="no (python >= ${PYTHON_REQUIRED} not found"])	
fi

if test "$enable_python" = "yes" -a "$found_python" != "yes"; then
	AC_MSG_ERROR([$found_python])
fi

AM_CONDITIONAL([ENABLE_PYTHON], [test "$found_python" = "yes"])

dnl =======
dnl Doxygen
dnl =======

dnl we only want XML for sphinx to use
DX_XML_FEATURE(ON)
DX_HTML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
dnl stub info for doxygen, since breathe will end up calling it directly
DX_INIT_DOXYGEN($PACKAGE_NAME, [Doxyfile], [source/doxygen])

AM_COND_IF([DX_COND_doc], [build_docs=yes], [build_docs=no])

dnl =================
dnl Substitution List
dnl =================

AC_CONFIG_FILES([
Makefile
src/Makefile
tools/Makefile
bindings/Makefile
doc/Makefile
])

AC_OUTPUT

dnl =======
dnl Summary
dnl =======

echo "

Configuration:

        Source code location   : $srcdir
        Compiler               : $CC
        Installation prefix    : $prefix
        Build documentation    : $build_docs

Language Bindings:

        Python : $found_python

    The source is now prepared.
    Type \`make' to build libredstone.
"
