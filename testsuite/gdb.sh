#!/bin/sh

# Usage:
# gdb <gdb_program> <debug_script> <openocd_script> <target_elf>

if [[ $# -lt 5 ]]; then
    cat << EOF
Usage:
$0 <gdb> <gdb_script> <openocd_script> <test_executable> <tmp_dir>

<gdb>             - path to GDB executable
<gdb_script>      - path to GDB script driving tests
<openocd_script>  - path to OpenOCD script
<text_executable> - path to text binary file (firmware)
<tmp_dir>         - directory for temporary files (PID file)
EOF
    exit 4
fi

if [ ! -x $1 ]; then
    echo $1: file is not an executable!
    exit 3
fi

if [ ! -f $2 ]; then
    echo $2: GDB script file does not exist!
    exit 3
fi

if [ ! -f $3 ]; then
    echo $3: OpenOCD script file does not exist!
    exit 3
fi

if [ ! -x $4 ]; then
    echo $4: Test file is not an executable!
    exit 3
fi

if [ ! -d $5 ]; then
    echo $5: Is not a directory!
    exit 3
fi

# run_openocd(openocd_script, tmp_dir)
function run_openocd() {
    openocd -f $1 &
    PID=$!
    echo $PID > $2/openocd.pid
    disown -h $PID
}

if [ ! -f $5/openocd.pid ]; then
    run_openocd $3 $5
else
    PID=`cat $5/openocd.pid`
    if ! ps -p $PID > /dev/null 
    then
        run_openocd $3 $5
    fi
fi

$1 -x $2 $4
