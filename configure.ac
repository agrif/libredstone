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
AX_FUNC_MKDIR

dnl ===================
dnl Memory Mapped Files
dnl ===================

AC_ARG_WITH(mmap, AS_HELP_STRING([--with-mmap], [set which mmap implementation to use (posix, windows, none)]), [], [with_mmap=auto])
if test "$with_mmap" = "no"; then
	with_mmap=none
fi

mmap_implementation=none

# CHECK_MMAP_IMPL(NAME, [CHECK CODE])
# -----------------------------------
AC_DEFUN([CHECK_MMAP_IMPL], [
	if test "$with_mmap" = $1 -o \( "$with_mmap" = "auto" -a "$mmap_implementation" = "none" \); then
		$2
	fi
])

CHECK_MMAP_IMPL("posix", [AC_CHECK_FUNC([mmap], [
	mmap_implementation="posix";
	AC_DEFINE([MMAP_POSIX], [], [Use the Posix mmap call.])
], [])])

if test "$mmap_implementation" = "none"; then
	AC_DEFINE([MMAP_NONE], [], [Use the terrible, inefficient mmap backend.])
fi

if test "$with_mmap" = "auto" -a "$mmap_implementation" = "none"; then
	AC_MSG_WARN([I cannot find an implementation of mmap, which libredstone])
	AC_MSG_WARN([uses to read files. To use an inefficient replacement, use])
	AC_MSG_WARN([--without-mmap as an argument to this script.])
	AC_MSG_ERROR([cannot find an mmap implementation])
fi

if test "$with_mmap" != "$mmap_implementation"; then
	AC_MSG_ERROR([cannot find mmap implementation '$with_mmap'. Please try another.])
fi

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

dnl ================
dnl Doxygen + Sphinx
dnl ================

AC_ARG_ENABLE(docs,
			  AS_HELP_STRING([--enable-docs], [Enable documentation]),
			  [enable_docs=$enableval],
			  [enable_docs=auto])

if test "$enable_docs" = "no"; then
    found_docs="no (disabled, use --enable-docs to enable)"
else
    dnl check for doxygen
    AC_CHECK_PROGS([DOXYGEN], [doxygen], [no])
    dnl check for sphinx
    AC_CHECK_PROGS([SPHINXBUILD], [sphinx-build sphinx-build3 sphinx-build2], [no])
    dnl check for breathe
    AC_MSG_CHECKING(for breathe sphinx extension)
    python -c "import breathe" > /dev/null 2>&1
    if test $? -ne 0 ; then
        found_breathe=no
    else
        found_breathe=yes
    fi
    AC_MSG_RESULT($found_breathe)
    
    if test "x$SPHINXBUILD" = "xno"; then
        found_docs="no (sphinx not found)"
    else
        if test "x$DOXYGEN" = "xno"; then
            found_docs="no (doxygen not found)"
        else
            if test "x$found_breathe" = "xno"; then
                found_docs="no (breathe not found)"
            else
                found_docs="yes"
            fi
        fi
    fi
fi

if test "$enable_docs" = "yes" -a "$found_docs" != "yes"; then
	AC_MSG_ERROR([$found_docs])
fi

AM_CONDITIONAL([ENABLE_DOCS], [test "$found_docs" = "yes"])

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

        Source code location : $srcdir
        Compiler             : $CC
        Installation prefix  : $prefix
        mmap implementation  : $mmap_implementation
        Build documentation  : $found_docs

Language Bindings:

        Python : $found_python

    The source is now prepared.
    Type \`make' to build libredstone.
"
