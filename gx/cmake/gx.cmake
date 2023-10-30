if (NOT GX_FIND_PATH)
    message(FATAL_ERROR "Not set: GX_FIND_PATH")
endif ()

if (WIN32)
    set(LINK_LIBRARY_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
    set(LINK_LIBRARY_PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX})
else ()
    set(LINK_LIBRARY_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(LINK_LIBRARY_PREFIX ${CMAKE_SHARED_LIBRARY_PREFIX})
endif ()

set(GX_INCLUDE_DIR ${GX_FIND_PATH}/include)
set(GX_BIN_DIR ${GX_FIND_PATH}/bin)
set(GX_LIB_DIR ${GX_FIND_PATH}/lib)

add_library(gx INTERFACE)
target_include_directories(gx INTERFACE ${GX_INCLUDE_DIR})
target_link_libraries(gx INTERFACE
        ${GX_LIB_DIR}/${LINK_LIBRARY_PREFIX}gx${LINK_LIBRARY_SUFFIX})

if (UNIX AND NOT APPLE AND NOT ANDROID)
    find_package(Threads)
    target_link_libraries(gx INTERFACE ${CMAKE_THREAD_LIBS_INIT})
endif ()

if (UNIX AND NOT APPLE AND NOT EMSCRIPTEN AND NOT ANDROID)
    find_library(RT_LIBRARY rt)
    mark_as_advanced(RT_LIBRARY)
    if (RT_LIBRARY)
        target_link_libraries(gx INTERFACE ${RT_LIBRARY})
    endif ()

    find_library(MATH_LIBRARY m)
    mark_as_advanced(MATH_LIBRARY)
    if (MATH_LIBRARY)
        target_link_libraries(gx INTERFACE ${MATH_LIBRARY})
    endif ()

    target_link_libraries(gx INTERFACE -latomic -ldl)
endif ()
