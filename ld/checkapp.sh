#!/bin/sh

if [ $# -ne 2 ]; then
    echo "Invalid arguments! Usage: $0 <file_name> <symbol_name>"
    exit 3
fi

if [ ! -e $1 ]; then
    echo "File $1 does not exist"
    exit 1
fi

SYMS=`objdump -j.applications -t $1 | grep $2 | wc -l`

if [ "${SYMS}" == "1" ]; then
    exit 0
else
    if [ "${SYMS}" == "0" ]; then
        echo "Symbol \`$2\` not found!"
    else 
        echo "Unexpected amount of symbol \`$2\` occurrences found!"
    fi
    exit 2
fi
