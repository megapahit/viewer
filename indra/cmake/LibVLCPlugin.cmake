# -*- cmake -*-
include(Linking)
include(Prebuilt)

include_guard()
add_library( ll::libvlc INTERFACE IMPORTED )

set(LIBVLCPLUGIN ON CACHE BOOL
        "LIBVLCPLUGIN support for the llplugin/llmedia test apps.")

if (LIBVLCPLUGIN)
    if(DARWIN)
        if (CMAKE_OSX_ARCHITECTURES MATCHES x86_64)
            set(ARCHITECTURE intel64)
        else ()
            set(ARCHITECTURE ${CMAKE_OSX_ARCHITECTURES})
        endif ()
        if (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/vlc_installed OR NOT ${vlc_installed} EQUAL 0)
            if (NOT EXISTS ${CMAKE_BINARY_DIR}/vlc-3.0.23-${ARCHITECTURE}.dmg)
                file(DOWNLOAD
                    https://get.videolan.org/vlc/3.0.23/macosx/vlc-3.0.23-${ARCHITECTURE}.dmg
                    ${CMAKE_BINARY_DIR}/vlc-3.0.23-${ARCHITECTURE}.dmg
                    )
            endif ()
            file(WRITE ${PREBUILD_TRACKING_DIR}/vlc_installed "0")
        endif ()
        execute_process(
            COMMAND hdiutil attach -noverify vlc-3.0.23-${ARCHITECTURE}.dmg
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            )
        target_include_directories( ll::libvlc SYSTEM INTERFACE /Volumes/VLC\ media\ player/VLC.app/Contents/MacOS/include)
        target_link_directories( ll::libvlc INTERFACE /Volumes/VLC\ media\ player/VLC.app/Contents/MacOS/lib)
        target_link_libraries( ll::libvlc INTERFACE vlc vlccore )
        return()
    elseif (NOT (${LINUX_DISTRO} MATCHES freedesktop OR WINDOWS))
        find_package(PkgConfig REQUIRED)

        pkg_check_modules(libvlc REQUIRED IMPORTED_TARGET libvlc vlc-plugin)
        target_include_directories( ll::libvlc SYSTEM INTERFACE ${libvlc_INCLUDE_DIRS})
        target_link_directories( ll::libvlc INTERFACE ${libvlc_LIBRARY_DIRS})
        target_link_libraries( ll::libvlc INTERFACE PkgConfig::libvlc)
        return()
    endif()

    use_prebuilt_binary(vlc-bin)
    if (WINDOWS)
        set(LIB_SUFFIX lib)
    endif ()
    target_link_libraries( ll::libvlc INTERFACE
            ${LIB_SUFFIX}vlc
            ${LIB_SUFFIX}vlccore
    )
    return()

    find_library(VLC_LIBRARY
        NAMES
        libvlc.lib
        libvlc.dylib
        PATHS "${ARCH_PREBUILT_DIRS_RELEASE}" REQUIRED NO_DEFAULT_PATH)

    find_library(VLCCORE_LIBRARY
        NAMES
        libvlccore.lib
        libvlccore.dylib
        PATHS "${ARCH_PREBUILT_DIRS_RELEASE}" REQUIRED NO_DEFAULT_PATH)

    target_link_libraries(ll::libvlc INTERFACE ${VLC_LIBRARY} ${VLCCORE_LIBRARY})
endif()
