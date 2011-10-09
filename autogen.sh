#!/bin/bash

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

if [ ! -f $srcdir/configure.ac -o ! -f $srcdir/README -o ! -d $srcdir/src ]; then
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level libredstone directory"
    exit 1
fi

source tests/colors.sh

ebegin "libtoolize"
libtoolize --copy
eend $?

ebegin "aclocal"
aclocal -I m4
eend $?

ebegin "autoconf"
autoconf
eend $?

ebegin "autoheader"
autoheader
eend $?

ebegin "automake"
automake --gnu --add-missing --copy
eend $?

#ebegin "configure"
./configure --enable-maintainer-mode $@
#eend $?
