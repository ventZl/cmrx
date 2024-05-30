#!/bin/sh

# Usage:
# gdb <gdb_program> <debug_script> <openocd_script> <target_elf>

echo GDB $1
echo GDB script $2
echo OOCD script $3
echo Text executable $4
echo Binary directory $5

PID=`cat $5/openocd.pid`
echo Is OpenOCD running as PID $PID ?
if ! ps -p $PID > /dev/null 
then
    openocd -f $3 &
    PID=$!
    echo $PID > $5/openocd.pid
    disown -h $PID
else
    echo OpenOCD already runs as PID $PID
fi

$1 -x $2 $4
