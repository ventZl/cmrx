set(pthread_SRCS mutex.c thread.c)

add_library(pthread STATIC ${pthread_SRCS})
