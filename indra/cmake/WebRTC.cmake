# -*- cmake -*-
include(Linking)
include(Prebuilt)

include_guard()

add_library( ll::webrtc INTERFACE IMPORTED )
target_include_directories( ll::webrtc SYSTEM INTERFACE "${LIBS_PREBUILT_DIR}/include/webrtc" "${LIBS_PREBUILT_DIR}/include/webrtc/third_party/abseil-cpp")
if (${LINUX_DISTRO} MATCHES debian OR CMAKE_OSX_ARCHITECTURES MATCHES x86_64 OR WINDOWS)
use_prebuilt_binary(webrtc)
elseif (NOT CMAKE_SYSTEM_NAME MATCHES FreeBSD)
    target_compile_definitions(ll::webrtc INTERFACE CM_WEBRTC=1)
    if (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/webrtc_installed OR NOT ${webrtc_installed} EQUAL 0)
        if (DARWIN)
            set(WEBRTC_PLATFORM macos-arm64)
        elseif (CMAKE_SYSTEM_PROCESSOR MATCHES aarch64)
            set(WEBRTC_PLATFORM linux-arm64)
        else ()
            set(WEBRTC_PLATFORM linux-x64)
        endif ()
        if (NOT EXISTS ${CMAKE_BINARY_DIR}/libwebrtc-${WEBRTC_PLATFORM}.tar.xz)
            file(DOWNLOAD
                https://github.com/crow-misia/libwebrtc-bin/releases/download/114.5735.6.1/libwebrtc-${WEBRTC_PLATFORM}.tar.xz
                ${CMAKE_BINARY_DIR}/libwebrtc-${WEBRTC_PLATFORM}.tar.xz
                SHOW_PROGRESS
                )
        endif ()
        file(ARCHIVE_EXTRACT
            INPUT ${CMAKE_BINARY_DIR}/libwebrtc-${WEBRTC_PLATFORM}.tar.xz
            DESTINATION ${LIBS_PREBUILT_DIR}
            )
        file(REMOVE_RECURSE ${LIBS_PREBUILT_DIR}/include/webrtc)
        file(MAKE_DIRECTORY ${LIBS_PREBUILT_DIR}/include/webrtc)
        foreach(directory
            api
            audio
            base
            build
            buildtools
            call
            common_audio
            common_video
            examples
            logging
            media
            modules
            net
            p2p
            pc
            rtc_base
            rtc_tools
            sdk
            stats
            system_wrappers
            test
            testing
            third_party
            tools
            video
            )
            file(RENAME
                ${LIBS_PREBUILT_DIR}/include/${directory}
                ${LIBS_PREBUILT_DIR}/include/webrtc/${directory}
                )
        endforeach()
        file(RENAME
            ${LIBS_PREBUILT_DIR}/lib/libwebrtc.a
            ${ARCH_PREBUILT_DIRS_RELEASE}/libwebrtc.a
            )
        if (CMAKE_OSX_ARCHITECTURES MATCHES arm64)
            file(REMOVE_RECURSE ${ARCH_PREBUILT_DIRS_RELEASE}/WebRTC.framework)
            file(RENAME
                ${LIBS_PREBUILT_DIR}/Frameworks/WebRTC.xcframework/${WEBRTC_PLATFORM}/WebRTC.framework
                ${ARCH_PREBUILT_DIRS_RELEASE}/WebRTC.framework
                )
            file(REMOVE_RECURSE ${LIBS_PREBUILT_DIR}/Frameworks)
        endif ()
        file(WRITE ${PREBUILD_TRACKING_DIR}/webrtc_installed "0")
    endif ()
endif ()

if (WINDOWS)
    target_link_libraries( ll::webrtc INTERFACE webrtc.lib )
elseif (DARWIN)
    FIND_LIBRARY(COREAUDIO_LIBRARY CoreAudio)
    FIND_LIBRARY(COREGRAPHICS_LIBRARY CoreGraphics)
    FIND_LIBRARY(AUDIOTOOLBOX_LIBRARY AudioToolbox)
    FIND_LIBRARY(COREFOUNDATION_LIBRARY CoreFoundation)
    FIND_LIBRARY(COCOA_LIBRARY Cocoa)

    target_link_libraries( ll::webrtc INTERFACE
        libwebrtc.a
        ${COREAUDIO_LIBRARY}
        ${AUDIOTOOLBOX_LIBRARY}
        ${COREGRAPHICS_LIBRARY}
        ${COREFOUNDATION_LIBRARY}
        ${COCOA_LIBRARY}
    )
elseif (LINUX)
    target_link_libraries( ll::webrtc INTERFACE libwebrtc.a X11 )
endif (WINDOWS)


