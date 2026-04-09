set(rp2xxx_SRCS
    rp2xxx.c
)

target_sources(os PRIVATE ${rp2xxx_SRCS})
target_include_directories(os PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
