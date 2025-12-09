#!/bin/sh

# Usage:
# gdb <gdb_program> <debug_script> <openocd_script> <target_elf>

if [[ $# -lt 5 ]]; then
    cat << EOF
Usage:
$0 <gdb> <gdb_init_script> <gdb_script> <openocd_script> <test_executable> <tmp_dir>

<gdb>             - path to GDB executable
<gdb_init_script> - path to GDB script that initializes communication with target
<gdb_script>      - path to GDB script driving tests
<openocd_script>  - path to OpenOCD script
<text_executable> - path to text binary file (firmware)
<tmp_dir>         - directory for temporary files (PID file)
EOF
    exit 4
fi

GDB_EXE=$1
GDB_INIT_SCRIPT=$2
GDB_TEST_SCRIPT=$3
OPENOCD_SCRIPT=$4
TEST_EXECUTABLE=$5
TMP_DIR=$6

if [ ! -x ${GDB_EXE} ]; then
    echo ${GDB_EXE}: file is not an executable!
    exit 3
fi

if [ ! -f ${GDB_INIT_SCRIPT} ]; then
    echo ${GDB_INIT_SCRIPT}: GDB target init script file does not exist!
    exit 3
fi

if [ ! -f ${GDB_TEST_SCRIPT} ]; then
    echo ${GDB_TEST_SCRIPT}: GDB test driver script file does not exist!
    exit 3
fi

if [ ! -f ${OPENOCD_SCRIPT} -a "${OPENOCD_SCRIPT}" != "-" ]; then
    echo ${OPENOCD_SCRIPT}: OpenOCD script file does not exist!
    exit 3
fi

if [ ! -x ${TEST_EXECUTABLE} ]; then
    echo ${TEST_EXECUTABLE}: Test file is not an executable!
    exit 3
fi

if [ ! -d ${TMP_DIR} ]; then
    echo ${TMP_DIR}: Is not a directory!
    exit 3
fi

# run_openocd(openocd_script, tmp_dir)
function run_openocd() {
    openocd -f $1 &
    PID=$!
    echo $PID > $2/openocd.pid
    disown -h $PID
}

if [ "${OPENOCD_SCRIPT}" != "-" ]; then
    if [ ! -f $5/openocd.pid ]; then
        run_openocd ${OPENOCD_SCRIPT} ${TMP_DIR}
    else
        PID=`cat ${TMP_DIR}/openocd.pid`
        if ! ps -p $PID > /dev/null
        then
            run_openocd ${OPENOCD_SCRIPT} ${TMP_DIR}
        fi
    fi
fi

unset DEBUGINFOD_URLS
echo ${GDB_EXE} -x ${GDB_INIT_SCRIPT} -x ${GDB_TEST_SCRIPT} ${TEST_EXECUTABLE}
${GDB_EXE} -x ${GDB_INIT_SCRIPT} -x ${GDB_TEST_SCRIPT} ${TEST_EXECUTABLE}
