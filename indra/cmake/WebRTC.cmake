# -*- cmake -*-
include_guard()

include(Linking)
include(Prebuilt)

add_library( ll::webrtc INTERFACE IMPORTED )
target_include_directories( ll::webrtc SYSTEM INTERFACE "${LIBS_PREBUILT_DIR}/include/webrtc" "${LIBS_PREBUILT_DIR}/include/webrtc/third_party/abseil-cpp")
if (DARWIN OR WINDOWS)
use_prebuilt_binary(webrtc)
elseif (NOT (CMAKE_SYSTEM_NAME MATCHES FreeBSD OR ($ENV{MSYSTEM_CARCH} MATCHES aarch64) OR (${LINUX_DISTRO} MATCHES debian AND CMAKE_SYSTEM_PROCESSOR MATCHES aarch64)))
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
                https://github.com/crow-misia/libwebrtc-bin/releases/download/137.7151.3.1/libwebrtc-${WEBRTC_PLATFORM}.tar.xz
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

find_library(WEBRTC_LIBRARY
    NAMES
    webrtc
    PATHS "${ARCH_PREBUILT_DIRS_RELEASE}" REQUIRED NO_DEFAULT_PATH)

target_link_libraries( ll::webrtc INTERFACE ${WEBRTC_LIBRARY} )

if (DARWIN)
    if (CMAKE_OSX_ARCHITECTURES MATCHES x86_64)
        target_link_directories( ll::webrtc INTERFACE ${ARCH_PREBUILT_DIRS_RELEASE} )
        target_link_libraries( ll::webrtc INTERFACE webrtc )
    endif ()
    target_link_libraries( ll::webrtc INTERFACE ll::oslibraries )
    execute_process(
        COMMAND lipo libwebrtc.a
            -thin ${CMAKE_OSX_ARCHITECTURES}
            -output libwebrtc.a
        WORKING_DIRECTORY ${ARCH_PREBUILT_DIRS_RELEASE}
    )
elseif (LINUX)
    target_link_libraries( ll::webrtc INTERFACE X11 )
endif ()


