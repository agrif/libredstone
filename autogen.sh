#!/bin/bash

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

if [ ! -f $srcdir/configure.ac -o ! -f $srcdir/README -o ! -d $srcdir/src ]; then
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level libredstone directory"
    exit 1
fi

COLS=${COLUMNS:-80}
LAST_LEN=0
LAST_BEGIN=""

GREEN='\e[32;01m'
RED='\e[31;01m'
BLUE='\e[34;01m'
NORMAL='\e[0m'

TMPFILE="autogen.log"

ebegin()
{
    msg=" * Running $@... "
    echo -n "$msg"
    LAST_LEN=${#msg}
    LAST_BEGIN="$@"

    rm -f $TMPFILE
    exec 3>$TMPFILE
    exec 4>&1
    exec 5>&2
    exec 1>&3
    exec 2>&3
}

eend()
{
    if [[ $1 == "0" ]]; then
        msg=" ${BLUE}[ ${GREEN}ok${BLUE} ]${NORMAL}"
    else
        msg=" ${BLUE}[ ${RED}!!${BLUE} ]${NORMAL}"
    fi
    
    exec 1>&4
    exec 2>&5
    exec 3>&-
    
    printf "%$(( COLS - LAST_LEN - 7 ))s%b\n" '' "$msg"
    
    if [[ $1 != "0" ]]; then
        printf "\n"
        cat $TMPFILE
        printf "\n ${RED}*${NORMAL} ${LAST_BEGIN} failed.\n"
        
        rm -f $TMPFILE
        exit $1
    fi
    
    rm -f $TMPFILE
}

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

ebegin "configure"
./configure --enable-maintainer-mode $@
eend $?

echo
echo "  The source is now prepared."
echo "  Type \`make' to build libredstone."
echo