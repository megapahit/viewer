# -*- cmake -*-
include(Linking)
include(Prebuilt)

include_guard()
add_library( ll::libvlc INTERFACE IMPORTED )

if (DARWIN)
    if (CMAKE_OSX_ARCHITECTURES MATCHES x86_64)
        set(ARCHITECTURE intel64)
    else ()
        set(ARCHITECTURE ${CMAKE_OSX_ARCHITECTURES})
    endif ()
    if (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/vlc_installed OR NOT ${vlc_installed} EQUAL 0)
        if (NOT EXISTS ${CMAKE_BINARY_DIR}/vlc-3.0.21-${ARCHITECTURE}.dmg)
            file(DOWNLOAD
                https://get.videolan.org/vlc/3.0.21/macosx/vlc-3.0.21-${ARCHITECTURE}.dmg
                ${CMAKE_BINARY_DIR}/vlc-3.0.21-${ARCHITECTURE}.dmg
                )
        endif ()
        file(WRITE ${PREBUILD_TRACKING_DIR}/vlc_installed "0")
    endif ()
    execute_process(
        COMMAND hdiutil attach -noverify vlc-3.0.21-${ARCHITECTURE}.dmg
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    target_include_directories( ll::libvlc SYSTEM INTERFACE /Volumes/VLC\ media\ player/VLC.app/Contents/MacOS/include)
    target_link_directories( ll::libvlc INTERFACE /Volumes/VLC\ media\ player/VLC.app/Contents/MacOS/lib)
    target_link_libraries( ll::libvlc INTERFACE vlc vlccore )
else ()
    include(FindPkgConfig)
    pkg_check_modules(Libvlc REQUIRED libvlc vlc-plugin)
    target_include_directories( ll::libvlc SYSTEM INTERFACE ${Libvlc_INCLUDE_DIRS} )
    target_link_directories( ll::libvlc INTERFACE ${Libvlc_LIBRARY_DIRS} )
    target_link_libraries( ll::libvlc INTERFACE ${Libvlc_LIBRARIES} )
endif ()

#use_prebuilt_binary(vlc-bin)
set(LIBVLCPLUGIN ON CACHE BOOL
        "LIBVLCPLUGIN support for the llplugin/llmedia test apps.")

return()

if (WINDOWS)
    target_link_libraries( ll::libvlc INTERFACE
            libvlc.lib
            libvlccore.lib
    )
elseif (DARWIN)
    target_link_libraries( ll::libvlc INTERFACE
            libvlc.dylib
            libvlccore.dylib
    )
elseif (LINUX)
    # Specify a full path to make sure we get a static link
    target_link_libraries( ll::libvlc INTERFACE
        ${LIBS_PREBUILT_DIR}/lib/libvlc.a
        ${LIBS_PREBUILT_DIR}/lib/libvlccore.a
    )
endif (WINDOWS)
