# -*- cmake -*-

project(media_plugin_cef)

include(Boost)
include(00-Common)
include(LLCommon)
include(LLWindow)
include(Linking)
include(PluginAPI)

include(CEFPlugin)
include(GLIB)

### media_plugin_cef

set(media_plugin_cef_SOURCE_FILES
    media_plugin_cef.cpp
    )

set(media_plugin_cef_HEADER_FILES
    volume_catcher.h
    )

# Select which VolumeCatcher implementation to use
if (LINUX)
  foreach( PULSE_FILE pulse/introspect.h pulse/context.h pulse/subscribe.h pulse/glib-mainloop.h )
    find_path( PULSE_FILE_${PULSE_FILE}_FOUND ${PULSE_FILE} NO_CACHE)
    if( NOT PULSE_FILE_${PULSE_FILE}_FOUND )
      message( "Looking for ${PULSE_FILE} ... not found")
      message( FATAL_ERROR "Pulse header not found" )
    else()
      message( "Looking for ${PULSE_FILE} ... found")
    endif()
  endforeach()
  
  include(FindPipeWire)
  include_directories(SYSTEM ${PIPEWIRE_INCLUDE_DIRS} ${SPA_INCLUDE_DIRS})
  
  message( "Building with Linux volume catcher for PipeWire and PulseAudio" )

  list(APPEND media_plugin_cef_HEADER_FILES
    linux/volume_catcher_linux.h
  )

  set(LINUX_VOLUME_CATCHER 
    linux/volume_catcher_linux.cpp 
    linux/volume_catcher_pulseaudio.cpp
    linux/volume_catcher_pipewire.cpp 
  )

  if (NOT (${LINUX_DISTRO} MATCHES debian OR (${LINUX_DISTRO} MATCHES ubuntu)))
    message( "Building with Linux volume catcher for PulseAudio only and cancelling PipeWire" )
    list(REMOVE_ITEM LINUX_VOLUME_CATCHER linux/volume_catcher_pipewire.cpp)
  endif ()

  list(APPEND media_plugin_cef_SOURCE_FILES ${LINUX_VOLUME_CATCHER})
  set(CMAKE_SHARED_LINKER_FLAGS  "-Wl,--build-id -Wl,-rpath,'$ORIGIN:$ORIGIN/../../lib'")
  list(APPEND media_plugin_cef_LINK_LIBRARIES llwindow )
elseif (DARWIN)
  list(APPEND media_plugin_cef_SOURCE_FILES volume_catcher_null.cpp)
  find_library(CORESERVICES_LIBRARY CoreServices)
  find_library(AUDIOUNIT_LIBRARY AudioUnit)
  set( media_plugin_cef_LINK_LIBRARIES
       ${CORESERVICES_LIBRARY}     # for Component Manager calls
       ${AUDIOUNIT_LIBRARY}        # for AudioUnit calls
       )
elseif (WINDOWS)
  list(APPEND media_plugin_cef_SOURCE_FILES windows_volume_catcher.cpp)
endif (LINUX)

list(APPEND media_plugin_cef_SOURCE_FILES ${media_plugin_cef_HEADER_FILES})

add_library(media_plugin_cef
    SHARED
    ${media_plugin_cef_SOURCE_FILES}
    )

if (${LINUX_DISTRO} MATCHES debian OR (${LINUX_DISTRO} MATCHES ubuntu))
    target_compile_definitions(media_plugin_cef PRIVATE USE_VOLUME_CATCHER_PW=1)
endif ()

#add_dependencies(media_plugin_cef
#  ${MEDIA_PLUGIN_BASE_LIBRARIES}
#)

target_link_libraries(media_plugin_cef
        media_plugin_base
        ll::cef
        ll::glib_headers
)

if (WINDOWS)
  set_target_properties(
    media_plugin_cef
    PROPERTIES
    LINK_FLAGS "/MANIFEST:NO /NODEFAULTLIB:LIBCMT /IGNORE:4099"
    )
endif (WINDOWS)

if (DARWIN)
  # Don't prepend 'lib' to the executable name, and don't embed a full path in the library's install name
  set_target_properties(
    media_plugin_cef
    PROPERTIES
    PREFIX ""
    BUILD_WITH_INSTALL_RPATH 1
    INSTALL_NAME_DIR "@executable_path"
    LINK_FLAGS "-exported_symbols_list ${CMAKE_CURRENT_SOURCE_DIR}/../base/media_plugin_base.exp"
  )

  add_custom_command(TARGET media_plugin_cef
    POST_BUILD COMMAND ${CMAKE_INSTALL_NAME_TOOL} -change "@executable_path/Chromium Embedded Framework"
        "@executable_path/../../../../Frameworks/Chromium Embedded Framework.framework/Chromium Embedded Framework"
        -change "/opt/local/lib/libpng16.16.dylib" "@loader_path/../../Frameworks/libpng16.16.dylib"
        -change "/opt/local/lib/libjpeg.8.dylib" "@loader_path/../../Frameworks/libjpeg.8.dylib"
        -change "/opt/local/lib/libfreetype.6.dylib" "@loader_path/../../Frameworks/libfreetype.6.dylib"
        -change "/opt/local/lib/libaprutil-1.0.dylib" "@loader_path/../../Frameworks/libaprutil-1.0.dylib"
        -change "/opt/local/lib/libiconv.2.dylib" "@loader_path/../../Frameworks/libiconv.2.dylib"
        -change "/opt/local/lib/libapr-1.0.dylib" "@loader_path/../../Frameworks/libapr-1.0.dylib"
        -change "/opt/local/lib/libexpat.1.dylib" "@loader_path/../../Frameworks/libexpat.1.dylib"
        -change "/opt/local/libexec/boost/1.87/lib/libboost_context-mt.dylib" "@loader_path/../../Frameworks/libboost_context-mt.dylib"
        -change "/opt/local/libexec/boost/1.87/lib/libboost_fiber-mt.dylib" "@loader_path/../../Frameworks/libboost_fiber-mt.dylib"
        -change "/opt/local/libexec/boost/1.87/lib/libboost_filesystem-mt.dylib" "@loader_path/../../Frameworks/libboost_filesystem-mt.dylib"
        -change "/opt/local/libexec/boost/1.87/lib/libboost_program_options-mt.dylib" "@loader_path/../../Frameworks/libboost_program_options-mt.dylib"
        -change "/opt/local/libexec/boost/1.87/lib/libboost_regex-mt.dylib" "@loader_path/../../Frameworks/libboost_regex-mt.dylib"
        -change "/opt/local/libexec/boost/1.87/lib/libboost_system-mt.dylib" "@loader_path/../../Frameworks/libboost_system-mt.dylib"
        -change "/opt/local/libexec/boost/1.87/lib/libboost_thread-mt.dylib" "@loader_path/../../Frameworks/libboost_thread-mt.dylib"
        -change "/opt/local/libexec/boost/1.87/lib/libboost_url-mt.dylib" "@loader_path/../../Frameworks/libboost_url-mt.dylib"
        -change "/opt/local/lib/libz.1.dylib" "@loader_path/../../Frameworks/libz.1.dylib"
        -change "/opt/local/lib/libnghttp2.14.dylib" "@loader_path/../../Frameworks/libnghttp2.14.dylib"
        "$<TARGET_FILE:media_plugin_cef>"
    VERBATIM
    COMMENT "Fixing path to CEF Framework"
  )

endif (DARWIN)

if (INSTALL)
    if (DARWIN)
        set(_LIB llplugin)
        install(
            DIRECTORY "${AUTOBUILD_INSTALL_DIR}/lib/release/Chromium Embedded Framework.framework"
            DESTINATION ../Frameworks
            )
        install(
            DIRECTORY
                "${AUTOBUILD_INSTALL_DIR}/lib/release/DullahanHelper.app"
                "${AUTOBUILD_INSTALL_DIR}/lib/release/DullahanHelper (GPU).app"
                "${AUTOBUILD_INSTALL_DIR}/lib/release/DullahanHelper (Plugin).app"
                "${AUTOBUILD_INSTALL_DIR}/lib/release/DullahanHelper (Renderer).app"
            DESTINATION SLPlugin.app/Contents/Frameworks
            )
    elseif (LINUX)
        if (${LINUX_DISTRO} MATCHES debian OR (${LINUX_DISTRO} MATCHES ubuntu))
            set(_LIB lib/${ARCH}-linux-gnu)
        elseif (${LINUX_DISTRO} MATCHES fedora OR (${LINUX_DISTRO} MATCHES opensuse-tumbleweed) OR (${LINUX_DISTRO} MATCHES gentoo))
            set(_LIB lib${ADDRESS_SIZE})
        else ()
            set(_LIB lib)
        endif ()
        if (${LINUX_DISTRO} MATCHES arch)
            install(
                PROGRAMS
                    ${AUTOBUILD_INSTALL_DIR}/bin/release/chrome-sandbox
                    ${AUTOBUILD_INSTALL_DIR}/bin/release/dullahan_host
                DESTINATION lib/${VIEWER_BINARY_NAME}
                #PERMISSIONS SETUID OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
            )
        else (${LINUX_DISTRO} MATCHES arch)
            install(
                PROGRAMS
                    ${AUTOBUILD_INSTALL_DIR}/bin/release/chrome-sandbox
                    ${AUTOBUILD_INSTALL_DIR}/bin/release/dullahan_host
                DESTINATION libexec/${VIEWER_BINARY_NAME}
                #PERMISSIONS SETUID OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
            )
        endif (${LINUX_DISTRO} MATCHES arch)
        install(
            FILES
                ${AUTOBUILD_INSTALL_DIR}/lib/release/libcef.so
                ${AUTOBUILD_INSTALL_DIR}/bin/release/snapshot_blob.bin
                ${AUTOBUILD_INSTALL_DIR}/bin/release/v8_context_snapshot.bin
                ${AUTOBUILD_INSTALL_DIR}/resources/chrome_100_percent.pak
                ${AUTOBUILD_INSTALL_DIR}/resources/chrome_200_percent.pak
                ${AUTOBUILD_INSTALL_DIR}/resources/icudtl.dat
                ${AUTOBUILD_INSTALL_DIR}/resources/resources.pak
            DESTINATION ${_LIB}
            )
        install(
            DIRECTORY ${AUTOBUILD_INSTALL_DIR}/resources/locales
            DESTINATION ${_LIB}
            )
    else (DARWIN)
        set(_LIB lib)
    endif (DARWIN)
    install(TARGETS ${PROJECT_NAME} DESTINATION ${_LIB})
endif (INSTALL)
