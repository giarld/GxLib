cmake_minimum_required(VERSION 3.20)

project(GX)

set(TARGET_NAME gx)

file(GLOB_RECURSE GX_CORE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

if (APPLE)
    list(APPEND GX_CORE_SRC
            ${CMAKE_CURRENT_SOURCE_DIR}/src/gfile.mm
            )
endif (APPLE)

add_library(${TARGET_NAME}
        ${GX_CORE_SRC}
        )
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${TARGET_NAME} PUBLIC gany-interface)
target_link_libraries(${TARGET_NAME} PRIVATE z tweetnacl)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(${TARGET_NAME} PUBLIC BUILD_SHARED_LIBS=1)
endif ()

if (MINGW)
    target_link_libraries(${TARGET_NAME} PRIVATE winmm)
endif ()

# Additional dependencies on Unix
if (UNIX AND NOT APPLE AND NOT ANDROID)
    # Threads
    find_package(Threads)
    target_link_libraries(${TARGET_NAME} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
endif ()

if (UNIX AND NOT APPLE AND NOT EMSCRIPTEN AND NOT ANDROID)
    find_library(RT_LIBRARY rt)
    mark_as_advanced(RT_LIBRARY)
    if (RT_LIBRARY)
        target_link_libraries(${TARGET_NAME} PUBLIC ${RT_LIBRARY})
    endif ()

    find_library(MATH_LIBRARY m)
    mark_as_advanced(MATH_LIBRARY)
    if (MATH_LIBRARY)
        target_link_libraries(${TARGET_NAME} PUBLIC ${MATH_LIBRARY})
    endif ()

    find_library(LibUUID_LIBRARY uuid)
    mark_as_advanced(LibUUID_LIBRARY)
    if (LibUUID_LIBRARY)
        target_link_libraries(${TARGET_NAME} PRIVATE ${LibUUID_LIBRARY})
    endif ()

    target_link_libraries(${TARGET_NAME} PUBLIC -latomic -ldl)
endif ()

if (ANDROID)
    target_link_libraries(${TARGET_NAME} PUBLIC log)
endif ()

if (APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    mark_as_advanced(COCOA_LIBRARY)
    target_link_libraries(${TARGET_NAME} PRIVATE ${COCOA_LIBRARY})
endif ()

if (CMAKE_COMPILER_IS_GNUCXX)
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-Wa,-mbig-obj" GNU_BIG_OBJ_FLAG_ENABLE)
    message(STATUS GNU_BIG_OBJ_FLAG_ENABLE=${GNU_BIG_OBJ_FLAG_ENABLE})
endif ()

target_compile_options(${TARGET_NAME}
        PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<BOOL:${GNU_BIG_OBJ_FLAG_ENABLE}>>:-Wa,-mbig-obj>)

# Doc
if (BUILD_GANY_DOC)
    add_custom_target(make-gx-doc
            COMMAND doc-make -o ${CMAKE_BINARY_DIR}/doc/${TARGET_NAME} -n Gx
            WORKING_DIRECTORY ${T_EXECUTABLE_OUTPUT_PATH}
            DEPENDS doc-make
    )
endif ()

### Install
if (GX_LIBS_INSTALL_DIR)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/gx DESTINATION ${GX_LIBS_INSTALL_DIR}/gx/include)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/gx.cmake DESTINATION ${GX_LIBS_INSTALL_DIR}/gx)
    install(TARGETS ${TARGET_NAME}
            RUNTIME DESTINATION ${GX_LIBS_INSTALL_DIR}/gx/bin
            LIBRARY DESTINATION ${GX_LIBS_INSTALL_DIR}/gx/lib
            ARCHIVE DESTINATION ${GX_LIBS_INSTALL_DIR}/gx/lib
            )
endif ()