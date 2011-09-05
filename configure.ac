dnl =============================
dnl Version Rules and AM/AC setup
dnl =============================

AC_PREREQ(2.63.2)

m4_define(libredstone_major_version, 0)
m4_define(libredstone_minor_version, 0)
m4_define(libredstone_patch_version, 0)

m4_define(libredstone_version, [libredstone_major_version.libredstone_minor_version.libredstone_patch_version])

AC_INIT([libredstone], [libredstone_version])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/redstone.h])
AC_CONFIG_MACRO_DIR([m4])

AC_DEFINE(LIBREDSTONE_MAJOR_VERSION, libredstone_major_version, [libredstone major version])
AC_SUBST(LIBREDSTONE_MAJOR_VERSION, libredstone_major_version)
AC_DEFINE(LIBREDSTONE_MINOR_VERSION, libredstone_minor_version, [libredstone minor version])
AC_SUBST(LIBREDSTONE_MINOR_VERSION, libredstone_minor_version)
AC_DEFINE(LIBREDSTONE_PATCH_VERSION, libredstone_patch_version, [libredstone patch version])
AC_SUBST(LIBREDSTONE_PATCH_VERSION, libredstone_patch_version)

AC_DEFINE(LIBREDSTONE_VERSION, "libredstone_version", [libredstone version])
AC_SUBST(LIBREDSTONE_VERSION, libredstone_version)

AM_INIT_AUTOMAKE([1.11 foreign dist-bzip2 no-dist-gzip])
AM_MAINTAINER_MODE([enable])

AM_SILENT_RULES([yes])

dnl =====================
dnl Basic Compiler Checks
dnl =====================

AC_PROG_CC
AC_PROG_INSTALL

LT_PREREQ([2.2.6])
LT_INIT

dnl =================
dnl Dependency Checks
dnl =================

AX_CHECK_ZLIB

dnl =======
dnl Doxygen
dnl =======

DX_HTML_FEATURE(ON)
DX_INIT_DOXYGEN($PACKAGE_NAME, [Doxyfile], [reference])

AM_COND_IF([DX_COND_doc], [build_docs=yes], [build_docs=no])

dnl =================
dnl Substitution List
dnl =================

AC_CONFIG_FILES([
Makefile
src/Makefile
tools/Makefile
doc/Makefile
doc/Doxyfile
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
"
