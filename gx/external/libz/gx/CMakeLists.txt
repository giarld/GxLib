cmake_minimum_required(VERSION 3.19)
project(z)

set(OUR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(TARGET_NAME         z)
set(SRC_DIR             ${OUR_DIR})
set(PUBLIC_HDR_DIR      ${OUR_DIR})
set(PRIVATE_HDR_DIR     ${OUR_DIR})

set(PUBLIC_HDRS
    ${PUBLIC_HDR_DIR}/zconf.h
    ${PUBLIC_HDR_DIR}/zlib.h
)

set(PRIVATE_HDRS
    ${PRIVATE_HDR_DIR}/crc32.h
    ${PRIVATE_HDR_DIR}/deflate.h
    ${PRIVATE_HDR_DIR}/gzguts.h
    ${PRIVATE_HDR_DIR}/inffast.h
    ${PRIVATE_HDR_DIR}/inffixed.h
    ${PRIVATE_HDR_DIR}/inflate.h
    ${PRIVATE_HDR_DIR}/inftrees.h
    ${PRIVATE_HDR_DIR}/trees.h
    ${PRIVATE_HDR_DIR}/zutil.h
)

set(SRCS
    ${SRC_DIR}/adler32.c
    ${SRC_DIR}/compress.c
    ${SRC_DIR}/crc32.c
    ${SRC_DIR}/deflate.c
    ${SRC_DIR}/gzclose.c
    ${SRC_DIR}/gzlib.c
    ${SRC_DIR}/gzread.c
    ${SRC_DIR}/gzwrite.c
    ${SRC_DIR}/inflate.c
    ${SRC_DIR}/infback.c
    ${SRC_DIR}/inftrees.c
    ${SRC_DIR}/inffast.c
    ${SRC_DIR}/trees.c
    ${SRC_DIR}/uncompr.c
    ${SRC_DIR}/zutil.c
)

if (NOT MSVC)
    add_definitions(
        -Wno-implicit-function-declaration
        -Wno-shift-negative-value
    )
endif()

add_library(${TARGET_NAME} STATIC ${PRIVATE_HDRS} ${PUBLIC_HDRS} ${SRCS})

target_include_directories (${TARGET_NAME} PUBLIC ${PUBLIC_HDR_DIR})
