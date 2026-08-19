#!/bin/sh

function test_fail() {
    echo "TEST FAILED: $1!"
    exit 1
}

function test_pass() {
    echo "TEST PASSED"
    return 0
}

# test_create_includes SOURCE_DIR BINARY_DIR LINKER_FILE
function test_create_includes() {
    TMP_DIR=$2/ld/tests/
    mkdir -p ${TMP_DIR}
    rm -rf ${TMP_DIR}/*
    python3 genlink-cmsis.py --create $1/tests/data/$3 ${TMP_DIR}/foo.$3 foo || test_fail "Error while running genlink-cmsis"
    for SECTION in bss data inst shared text vtable; do
        grep "INCLUDE gen.foo.${SECTION}.ld" ${TMP_DIR}/foo.$3 > /dev/null || test_fail ".${SECTION} section include directive not present"
        [ -e ${TMP_DIR}/gen.foo.${SECTION}.ld ] || test_fail ".${SECTION} section include file does not exit"
    done
    
}

if [ "$#" -lt "2" ]; then
    echo "$0: <source_dir> <binary_dir>"
    exit 1
fi

for LD_FILE in rp2040_default.ld rp2350_default.ld STM32G491CCUx_FLASH.ld stm32h753vihx_flash.ld max32690_rom.ld; do
    test_create_includes $1 $2 ${LD_FILE}
    TEST_OUTCOME=$?
    if [ ${TEST_OUTCOME} != 0 ]; then
	exit ${TEST_OUTCOME}
    fi
done

test_pass