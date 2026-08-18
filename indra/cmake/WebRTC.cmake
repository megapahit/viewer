# -*- cmake -*-
include_guard()

include(Linking)
include(Prebuilt)

add_library( ll::webrtc INTERFACE IMPORTED )
target_include_directories( ll::webrtc SYSTEM INTERFACE "${LIBS_PREBUILT_DIR}/include/webrtc" "${LIBS_PREBUILT_DIR}/include/webrtc/third_party/abseil-cpp")
if (CMAKE_SYSTEM_PROCESSOR MATCHES aarch64 AND LINUX)
    target_compile_definitions(ll::webrtc INTERFACE CM_WEBRTC=1)
    if (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/webrtc_installed OR NOT ${webrtc_installed} EQUAL 0)
        if (NOT EXISTS ${CMAKE_BINARY_DIR}/libwebrtc-linux-arm64.tar.xz)
            file(DOWNLOAD
                https://github.com/crow-misia/libwebrtc-bin/releases/download/139.7258.3.1/libwebrtc-linux-arm64.tar.xz
                ${CMAKE_BINARY_DIR}/libwebrtc-linux-arm64.tar.xz
                SHOW_PROGRESS
                )
        endif ()
        file(ARCHIVE_EXTRACT
            INPUT ${CMAKE_BINARY_DIR}/libwebrtc-linux-arm64.tar.xz
            DESTINATION ${LIBS_PREBUILT_DIR}
            )
        file(REMOVE_RECURSE ${LIBS_PREBUILT_DIR}/include/webrtc)
        file(MAKE_DIRECTORY ${LIBS_PREBUILT_DIR}/include/webrtc)
        foreach(directory
            api
            audio
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
        file(WRITE ${PREBUILD_TRACKING_DIR}/webrtc_installed "0")
    endif ()
elseif (NOT (CMAKE_SYSTEM_NAME MATCHES FreeBSD OR ($ENV{MSYSTEM_CARCH} MATCHES aarch64)))
use_prebuilt_binary(webrtc)
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


