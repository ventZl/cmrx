# Handle generation of unit test build
# Unit test build of CMRX kernel is a nested build created automatically
# within normal CMRX-based project binary directory, if -DUNIT_TESTING=1
# is passed to the commandline.
#
# If this happens, then the result are two binary directories:
# One configured for firmware build which is able to build HIL tests
# Another nested, configured for native build, which is able to build unit tests
#
# This combo is handled by following code

if (UNIT_TESTING)
    if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
        message(STATUS "Unit testing build enabled manually!")
        # This is special case handling, when UNIT_TESTING is requested directly on 
        # CMakeLists.txt of CMRX source tree. In this case we will synthesize __UNIT_TESTING_BUILD
        # as if it was called by the parent CMake.

        # This can not be merged with code below!
        set(__UNIT_TESTING_BUILD 1)
    endif()
endif()

if (__UNIT_TESTING_BUILD)
    if (NOT ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}"))
        message(FATAL_ERROR "Unit testing build creation has been requested but CMRX is not the topmost CMakeLists.txt! This is not supported!")
    endif()
    set(UNIT_TESTING_BUILD 1)
    set(UNIT_TESTING 1)
endif()

if (UNIT_TESTING)
    # Unit testing has been requested by the user

    if (NOT ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}"))
        # CMRX top level CMakeLists.txt is not whole project's top level CMakeLists.
        # This means, CMRX has been included by someone else.
        # In this case we will run CMake to create nested binary directory which
        # will hold the unit test build.

        if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
            # Future me created folder named `tests` and that's gonna be a problem
            message(WARNING "There is a directory named `tests` in CMRX source tree. Its content is going to be mixed with unit test build. Result is unpredictable!")
        endif()

        # Collect all non-default options
        set(UT_BUILD_OPTIONS )
        foreach(CMRX_OPTION ${CMRX_ALL_OPTIONS})
            message("Examining option ${CMRX_OPTION}")
            if (DEFINED ${CMRX_OPTION} AND NOT DEFINED CACHE{${CMRX_OPTION}})
                list(APPEND UT_BUILD_OPTIONS -D${CMRX_OPTION}=${${CMRX_OPTION}})
                message("Non default value for option ${CMRX_OPTION}")
            else()
                message("Option ${CMRX_OPTION} has default value ${${CMRX_OPTION}}")
            endif()
        endforeach()
        # Create directory which will host the build and run CMake
        file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
        execute_process(COMMAND ${CMAKE_COMMAND} -D__UNIT_TESTING_BUILD=1 ${UT_BUILD_OPTIONS} ${CMAKE_CURRENT_SOURCE_DIR}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
        message(Execute COMMAND ${CMAKE_COMMAND} -D__UNIT_TESTING_BUILD=1 ${UT_BUILD_OPTIONS} ${CMAKE_CURRENT_SOURCE_DIR}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests)


        # Create targets which jump into the nested build
        # Target to build all tests
        add_custom_target(build_tests ALL COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/tests)

        # Target to run all unit tests
        add_custom_target(unit_tests COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/tests -- test)

        # Register unit tests as a test in top level test suite
        add_test(NAME unit_tests COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/tests -- test)
    endif()
endif()
 
