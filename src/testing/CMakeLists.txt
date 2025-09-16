add_library(ctest STATIC EXCLUDE_FROM_ALL ctest_main.c)
target_include_directories(ctest PUBLIC ${ctest_SOURCE_DIR})
target_compile_options(ctest PUBLIC -Wno-attributes)
