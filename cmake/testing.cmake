# Find tests in given directory and process them as HIL tests
# 
# This routine will create one HW test for each subdirectory found
# in given directory. Tests are created based on directory content.
# See `make_hw_test` for more info.
#
# Tests can be augmented with platform file, if platform needs it.
# This may happen in case of SMP-enabled platform where some primitives
# need to be provided.
#
# Arguments:
# DIR - directory which contains subdirectories holding tests
# [PLATFORM_FILE] - optional platform source file included in the build
function(find_tests DIR) # optionally [PLATFORM_FILE]
    file(GLOB TESTS LIST_DIRECTORIES TRUE CONFIGURE_DEPENDS *)
    foreach(TEST ${TESTS})
        message(DEBUG "Assesing ${TEST}")
        if (IS_DIRECTORY ${TEST})
            make_hw_test(${TEST} ${ARGN})
        endif()
    endforeach()
endfunction()

# Create HIL test
#
# Registers a test that will run firmware via HIL test harness using GDB and OpenOCD.
# Harness will execute GDB with scripts that will load firmware into target, create breakpoints on
# function that mark test pass or fail and then execute the binary expecting some of breakpoints to
# be hit. The result is reported to the test orchestration tool.
#
# Arguments:
# TEST_NAME - name of the test as displayed in the test results
# FIRMWARE_NAME - CMake target containing firmware for the test
function(add_hil_test TEST_NAME FIRMWARE_NAME)
    if (NOT CMRX_HIL_TESTS)
        return()
    endif()

    message(STATUS "Adding test ${TEST_NAME}")
    get_property(CMRX_ROOT_DIR GLOBAL PROPERTY CMRX_ROOT_DIR)
    if (NOT TARGET ${FIRMWARE_NAME})
        message(FATAL_ERROR "${FIRMWARE_NAME}: Not a valid firmware name! Must be CMake target name!")
    endif()

    add_test(
        NAME ${TEST_NAME}
        COMMAND ${CMRX_ROOT_DIR}/testsuite/gdb.sh ${CMRX_GDB_PATH} ${GDB_INIT_FILE} ${GDB_FILE} ${OPENOCD_FILE} $<TARGET_FILE:${FIRMWARE_NAME}> ${CMAKE_BINARY_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(${TEST_NAME} PROPERTIES TIMEOUT 10)
endfunction()

# Assemble HW integration test
#
# This creates binary for a kernel HIL test. Binary is based on files contained
# in the root directory from which test search was initiated and from the directory
# content itself.
#
# Arguments:
# TEST_DIR - directory which contains the test
# [PLATFORM_FILE] - optional platform source file included into the build of test
function(make_hw_test TEST_DIR) # optionally [PLATFORM_FILE]
    if ("${CMRX_GDB_PATH}" STREQUAL "")
        message(FATAL_ERROR "variable CMRX_GDB_PATH not set. Tests can't execute")
    endif()
    # Some defaults used if not overriden by test itself
    set(MAIN_FILE main.c)
    set(HARNESS_FILE debug.c)
    set(PLATFORM_FILE ${ARGN})
    set(GDB_FILE ${CMAKE_CURRENT_LIST_DIR}/debug.gdb)
    set(GDB_INIT_FILE ${CMAKE_SOURCE_DIR}/test_init.gdb)

    if (NOT CMRX_HIL_TESTING_SKIP_OPENOCD)
        set(OPENOCD_FILE ${CMAKE_SOURCE_DIR}/openocd.cfg)
    else()
        set(OPENOCD_FILE "-")
    endif()

    set(TEST_APPS "")

    # Derive the test name := test directory name
    get_filename_component(TEST_NAME ${TEST_DIR} NAME)

    # Collect all test files
    file(GLOB TEST_FILES RELATIVE ${CMAKE_CURRENT_LIST_DIR} CONFIGURE_DEPENDS ${TEST_DIR}/*.c ${TEST_DIR}/*.h )

    # Figure out what is what?
    foreach(TEST_FILE ${TEST_FILES})
        get_filename_component(FILE_NAME ${TEST_FILE} NAME)
        # main.c will override the default main file
        if ("${FILE_NAME}" STREQUAL "main.c")
            set(MAIN_FILE ${TEST_FILE})
        else()
            # debug.gdb will override the default GDB file
            if ("${FILE_NAME}" STREQUAL "debug.gdb")
                set(GDB_FILE ${TEST_FILE})
            else()
                # Any other .c file is expected to be an application
                get_filename_component(FILE_EXT "${TEST_FILE}" EXT)
                if ("${FILE_EXT}" STREQUAL ".c")
                    get_filename_component(APP_NAME ${TEST_FILE} NAME_WE)
                    list(APPEND TEST_APPS "${APP_NAME}")
                    set(${APP_NAME} "${TEST_FILE}")
                endif()
            endif()
        endif()
    endforeach()
    if ("${TEST_APPS}" STREQUAL "")
        return()
    endif()
    add_firmware(${TEST_NAME} ${MAIN_FILE} ${TEST_FILE} ${PLATFORM_FILE})
    target_link_libraries(${TEST_NAME} hil_test_harness)
    target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

    foreach(APP ${TEST_APPS})
        set(APP_NAME ${TEST_NAME}_${APP})
        set(APP_SRCS ${${APP}})
        message(DEBUG "\t${TEST_NAME} has application ${APP_NAME}")
        add_application(${APP_NAME} ${APP_SRCS})
        target_include_directories(${APP_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
        target_link_libraries(${APP_NAME} os stdlib test_platform)
        #target_link_libraries(${APP_NAME} os)
        target_add_applications(${TEST_NAME} ${APP_NAME})
    endforeach()

    target_link_libraries(${TEST_NAME} test_platform_main)
    add_hil_test(${TEST_NAME} ${TEST_NAME})
endfunction()
