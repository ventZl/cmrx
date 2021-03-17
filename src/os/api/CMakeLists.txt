set(pthread_SRCS mutex.c thread.c)

add_library(pthread STATIC EXCLUDE_FROM_ALL ${pthread_SRCS})
