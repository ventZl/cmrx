set(pthreads_SRCS mutex.c)

add_library(pthreads STATIC ${pthreads_SRCS})
