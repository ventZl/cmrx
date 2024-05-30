include(CTest)

function(find_tests DIR)
    file(GLOB TESTS LIST_DIRECTORIES TRUE CONFIGURE_DEPENDS *)
    foreach(TEST ${TESTS})
        message(STATUS "Assesing ${TEST}")
        if (IS_DIRECTORY ${TEST})
            make_hw_test(${TEST})
        endif()
    endforeach()
endfunction()

function(make_hw_test TEST_DIR)
    if ("${CMRX_GDB_PATH}" STREQUAL "")
        message(FATAL_ERROR "variable CMRX_GDB_PATH not set. Tests can't execute")
    endif()
    # Some defaults used if not overriden by test itself
    set(MAIN_FILE main.c)
    set(HARNESS_FILE debug.c)
    set(GDB_FILE ${CMAKE_CURRENT_LIST_DIR}/debug.gdb)
    set(OPENOCD_FILE ${CMAKE_SOURCE_DIR}/openocd.cfg)
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
    message(STATUS "Adding test ${TEST_NAME}")
    add_firmware(${TEST_NAME} ${MAIN_FILE} ${TEST_FILE} ${HARNESS_FILE})
    target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

    foreach(APP ${TEST_APPS})
        set(APP_NAME ${TEST_NAME}_${APP})
        set(APP_SRCS ${${APP}})
        message(STATUS "\t${TEST_NAME} has application ${APP_NAME}")
        add_application(${APP_NAME} ${APP_SRCS})
        target_include_directories(${APP_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
        target_link_libraries(${APP_NAME} os stdlib aux_systick test_platform)
        #target_link_libraries(${APP_NAME} os)
        target_add_applications(${TEST_NAME} ${APP_NAME})
    endforeach()

    target_link_libraries(${TEST_NAME} test_platform_main)
    message(STATUS "Added test ${TEST_NAME}")
    message("${CMAKE_CURRENT_LIST_DIR}/gdb.sh ${CMRX_GDB_PATH} ${GDB_FILE} ${OPENOCD_FILE} $<TARGET_FILE:${TEST_NAME}> ${CMAKE_BINARY_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}")
    add_test(NAME ${TEST_NAME}
        COMMAND ${CMAKE_CURRENT_LIST_DIR}/gdb.sh ${CMRX_GDB_PATH} ${GDB_FILE} ${OPENOCD_FILE} $<TARGET_FILE:${TEST_NAME}> ${CMAKE_BINARY_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    set_tests_properties(${TEST_NAME} PROPERTIES TIMEOUT 10)
endfunction()

message(STATUS "TESTING ENABLED")
