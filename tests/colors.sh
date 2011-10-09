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
    
    printf "%$(( COLS - LAST_LEN - 8 ))s%b\n" '' "$msg"
    
    if [[ $1 != "0" ]]; then
        printf "\n"
        cat $TMPFILE
        printf "\n ${RED}*${NORMAL} ${LAST_BEGIN} failed.\n"
        
        if [[ $2 != "noexit" ]]; then
            rm -f $TMPFILE
            exit $1
        fi
    fi
    
    rm -f $TMPFILE
}
