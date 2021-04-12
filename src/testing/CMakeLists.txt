set(testing_SRCS intrinsics.c)

add_library(cmrx_testing STATIC EXCLUDE_FROM_ALL ${testing_SRCS})
