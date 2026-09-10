# -*- cmake -*-
include_guard()
include(Linking)
include(Prebuilt)

add_library(ll::SDL3 INTERFACE IMPORTED)

if (NOT (WINDOWS OR DARWIN))
  set(USE_SDL_WINDOW ON CACHE BOOL "Build with SDL window backend")
else()
  set(USE_SDL_WINDOW OFF CACHE BOOL "Build with SDL window backend")
endif ()

if(USE_SDL_WINDOW)
    #use_system_binary(SDL3)
    if (${LINUX_DISTRO} MATCHES debian)
        if (CMAKE_SYSTEM_PROCESSOR MATCHES x86_64)
    use_prebuilt_binary(SDL3)
        elseif (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/SDL3_installed OR NOT ${SDL3_installed} EQUAL 0)
            if (NOT EXISTS ${CMAKE_BINARY_DIR}/SDL-release-3.2.24.tar.gz)
                file(DOWNLOAD
                    https://github.com/libsdl-org/SDL/archive/refs/tags/release-3.2.24.tar.gz
                    ${CMAKE_BINARY_DIR}/SDL-release-3.2.24.tar.gz
                )
            endif ()
            file(ARCHIVE_EXTRACT
                INPUT ${CMAKE_BINARY_DIR}/SDL-release-3.2.24.tar.gz
                DESTINATION ${CMAKE_BINARY_DIR}
            )
            try_compile(SDL3_RESULT
                PROJECT SDL3
                SOURCE_DIR ${CMAKE_BINARY_DIR}/SDL-release-3.2.24
                BINARY_DIR ${CMAKE_BINARY_DIR}/SDL-release-3.2.24_build
                CMAKE_FLAGS
                    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                    -DCMAKE_INSTALL_PREFIX:PATH=${LIBS_PREBUILT_DIR}
                    -DCMAKE_INSTALL_LIBDIR:PATH=${ARCH_PREBUILT_DIRS_RELEASE}
                    -DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=ON
            )
            if (${SDL3_RESULT})
                execute_process(
                    COMMAND ${CMAKE_MAKE_PROGRAM} install
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/SDL-release-3.2.24_build
                    RESULT_VARIABLE SDL3_installed
                )
                file(WRITE ${PREBUILD_TRACKING_DIR}/SDL3_installed "${SDL3_installed}")
            endif ()
        endif ()

    find_library( SDL3_LIBRARY
        NAMES SDL3 SDL3.lib libSDL3.so libSDL3.dylib
        PATHS "${LIBS_PREBUILT_DIR}/lib/release" REQUIRED)

    target_link_libraries(ll::SDL3 INTERFACE ${SDL3_LIBRARY})
    target_include_directories(ll::SDL3 SYSTEM INTERFACE "${LIBS_PREBUILT_DIR}/include/")

    else ()
        include(FindPkgConfig)
        pkg_search_module(SDL3 REQUIRED sdl3)
        target_link_directories(ll::SDL3 INTERFACE ${SDL3_LIBRARY_DIRS})
        target_link_libraries(ll::SDL3 INTERFACE ${SDL3_LIBRARIES})
        target_include_directories(ll::SDL3 SYSTEM INTERFACE ${SDL3_INCLUDE_DIRS})
    endif ()
endif(USE_SDL_WINDOW)

