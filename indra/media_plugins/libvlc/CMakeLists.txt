# -*- cmake -*-

project(media_plugin_libvlc)

include(00-Common)
include(LLCommon)
include(LLImage)
include(LLWindow)
include(Linking)
include(PluginAPI)

include(LibVLCPlugin)

### media_plugin_libvlc


set(media_plugin_libvlc_SOURCE_FILES
    media_plugin_libvlc.cpp
    )

add_library(media_plugin_libvlc
    SHARED
    ${media_plugin_libvlc_SOURCE_FILES}
    )

target_link_libraries(media_plugin_libvlc
        media_plugin_base
        ll::libvlc
)

if (WINDOWS)
  set_target_properties(
    media_plugin_libvlc
    PROPERTIES
    LINK_FLAGS "/MANIFEST:NO /NODEFAULTLIB:LIBCMT"
    )
endif (WINDOWS)

if (DARWIN)
  # Don't prepend 'lib' to the executable name, and don't embed a full path in the library's install name
  set_target_properties(
    media_plugin_libvlc
    PROPERTIES
    PREFIX ""
    BUILD_WITH_INSTALL_RPATH 1
    INSTALL_RPATH "@executable_path/../Frameworks"
    LINK_FLAGS "-exported_symbols_list ${CMAKE_CURRENT_SOURCE_DIR}/../base/media_plugin_base.exp"
  )

  add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD COMMAND ${CMAKE_INSTALL_NAME_TOOL}
        -change "/opt/local/lib/libnghttp2.14.dylib" "@loader_path/libnghttp2.14.dylib"
        -change "/opt/local/lib/libaprutil-1.0.dylib" "@loader_path/libaprutil-1.0.dylib"
        -change "/opt/local/lib/libiconv.2.dylib" "@loader_path/libiconv.2.dylib"
        -change "/opt/local/lib/libapr-1.0.dylib" "@loader_path/libapr-1.0.dylib"
        -change "/opt/local/lib/libz.1.dylib" "@loader_path/libz.1.dylib"
        -change "/opt/local/libexec/boost/1.88/lib/libboost_context-mt.dylib" "@loader_path/libboost_context-mt.dylib"
        -change "/opt/local/libexec/boost/1.88/lib/libboost_fiber-mt.dylib" "@loader_path/libboost_fiber-mt.dylib"
        -change "/opt/local/libexec/boost/1.88/lib/libboost_filesystem-mt.dylib" "@loader_path/libboost_filesystem-mt.dylib"
        -change "/opt/local/libexec/boost/1.88/lib/libboost_program_options-mt.dylib" "@loader_path/libboost_program_options-mt.dylib"
        -change "/opt/local/libexec/boost/1.88/lib/libboost_thread-mt.dylib" "@loader_path/libboost_thread-mt.dylib"
        -change "/opt/local/libexec/boost/1.88/lib/libboost_url-mt.dylib" "@loader_path/libboost_url-mt.dylib"
        -change "/opt/local/libexec/boost/1.88/lib/libboost_regex-mt.dylib" "@loader_path/libboost_regex-mt.dylib"
        -change "/opt/local/libexec/boost/1.88/lib/libboost_system-mt.dylib" "@loader_path/libboost_system-mt.dylib"
        -change "/opt/local/lib/libexpat.1.dylib" "@loader_path/libexpat.1.dylib"
        -change "@rpath/libvlc.dylib" "@loader_path/plugins/libvlc.dylib"
        -change "@rpath/libvlccore.dylib" "@loader_path/plugins/libvlccore.dylib"
        "$<TARGET_FILE:${PROJECT_NAME}>"
    VERBATIM
    COMMENT "Fixing paths to LibVLC media plugin dependencies"
  )

endif (DARWIN)

if (INSTALL)
    if (DARWIN)
        set(_LIB SLPlugin.app/Contents/Frameworks)
        install(
            DIRECTORY /Volumes/VLC\ media\ player/VLC.app/Contents/MacOS/plugins
            DESTINATION ${_LIB}
            )
        install(
            FILES
                /Volumes/VLC\ media\ player/VLC.app/Contents/MacOS/lib/libvlc.5.dylib
                /Volumes/VLC\ media\ player/VLC.app/Contents/MacOS/lib/libvlc.dylib
                /Volumes/VLC\ media\ player/VLC.app/Contents/MacOS/lib/libvlccore.9.dylib
                /Volumes/VLC\ media\ player/VLC.app/Contents/MacOS/lib/libvlccore.dylib
            DESTINATION ${_LIB}/plugins
            )
    elseif (${LINUX_DISTRO} MATCHES debian OR (${LINUX_DISTRO} MATCHES ubuntu))
        set(_LIB lib/${ARCH}-linux-gnu)
    elseif (${LINUX_DISTRO} MATCHES fedora OR (${LINUX_DISTRO} MATCHES opensuse-tumbleweed) OR (${LINUX_DISTRO} MATCHES gentoo))
        set(_LIB lib${ADDRESS_SIZE})
    else ()
        set(_LIB lib)
    endif ()
    if (WINDOWS)
      install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${PROJECT_NAME}.dll DESTINATION llplugin)
      install(DIRECTORY ${AUTOBUILD_INSTALL_DIR}/bin/release/plugins DESTINATION llplugin)
    else ()
      install(TARGETS ${PROJECT_NAME} DESTINATION ${_LIB})
    endif ()
endif ()
