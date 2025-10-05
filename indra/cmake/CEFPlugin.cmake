# -*- cmake -*-
include(Linking)
include(Prebuilt)

include_guard()
add_library( ll::cef INTERFACE IMPORTED )

if (CMAKE_SYSTEM_PROCESSOR MATCHES aarch64)
    if (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/dullahan_installed OR NOT ${dullahan_installed} EQUAL 0)
        if (NOT EXISTS ${CMAKE_BINARY_DIR}/v1.14.0-r3.tar.gz)
            file(DOWNLOAD
                https://github.com/secondlife/dullahan/archive/refs/tags/v1.14.0-r3.tar.gz
                ${CMAKE_BINARY_DIR}/v1.14.0-r3.tar.gz
                )
        endif ()
        file(ARCHIVE_EXTRACT
            INPUT ${CMAKE_BINARY_DIR}/v1.14.0-r3.tar.gz
            DESTINATION ${CMAKE_BINARY_DIR}
            )
        execute_process(
            COMMAND sed -i "/#include <vector>/a #include <cstdint>" dullahan.h
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/dullahan-1.14.0-r3/src
            )
        file(MAKE_DIRECTORY ${LIBS_PREBUILT_DIR}/include/cef)
        try_compile(DULLAHAN_RESULT
            PROJECT dullahan
            SOURCE_DIR ${CMAKE_BINARY_DIR}/dullahan-1.14.0-r3
            BINARY_DIR ${CMAKE_BINARY_DIR}/dullahan-1.14.0-r3
            CMAKE_FLAGS
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                -DCMAKE_OSX_ARCHITECTURES:STRING=${CMAKE_OSX_ARCHITECTURES}
                -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=${CMAKE_OSX_DEPLOYMENT_TARGET}
                -DCMAKE_INSTALL_PREFIX:PATH=${LIBS_PREBUILT_DIR}
                -DCMAKE_INSTALL_LIBDIR:PATH=${ARCH_PREBUILT_DIRS_RELEASE}
                -DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=ON
                -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
                -DUSE_SPOTIFY_CEF:BOOL=ON
                -DSPOTIFY_CEF_URL:STRING=https://cef-builds.spotifycdn.com/cef_binary_118.4.1%2Bg3dd6078%2Bchromium-118.0.5993.54_linuxarm64_beta_minimal.tar.bz2
                -DPROJECT_ARCH:STRING=${CMAKE_SYSTEM_PROCESSOR}
                -DENABLE_CXX11_ABI:BOOL=ON
        )
        if (${DULLAHAN_RESULT})
            execute_process(
                COMMAND ${CMAKE_MAKE_PROGRAM} install
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/dullahan-1.14.0-r3
                OUTPUT_VARIABLE dullahan_installed
            )
            file(
                COPY
                    ${CMAKE_BINARY_DIR}/dullahan-1.14.0-r3/src/dullahan.h
                    ${CMAKE_BINARY_DIR}/dullahan-1.14.0-r3/src/dullahan_version.h
                DESTINATION ${LIBS_PREBUILT_DIR}/include/cef
                )
            file(WRITE ${PREBUILD_TRACKING_DIR}/dullahan_installed "${dullahan_installed}")
        endif (${DULLAHAN_RESULT})
    endif (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/dullahan_installed OR NOT ${dullahan_installed} EQUAL 0)
else ()
use_prebuilt_binary(dullahan)
endif ()

execute_process(
    COMMAND patchelf --remove-rpath bin/release/dullahan_host
    WORKING_DIRECTORY ${LIBS_PREBUILT_DIR}
    )

target_include_directories( ll::cef SYSTEM INTERFACE  ${LIBS_PREBUILT_DIR}/include/cef)

if (WINDOWS)
    target_link_libraries( ll::cef INTERFACE
        libcef.lib
        libcef_dll_wrapper.lib
        dullahan.lib
    )
elseif (DARWIN)
    FIND_LIBRARY(APPKIT_LIBRARY AppKit)
    if (NOT APPKIT_LIBRARY)
        message(FATAL_ERROR "AppKit not found")
    endif()

    set(CEF_LIBRARY "'${ARCH_PREBUILT_DIRS_RELEASE}/Chromium\ Embedded\ Framework.framework'")
    if (NOT CEF_LIBRARY)
        message(FATAL_ERROR "CEF not found")
    endif()

    target_link_libraries( ll::cef INTERFACE
        ${ARCH_PREBUILT_DIRS_RELEASE}/libcef_dll_wrapper.a
        ${ARCH_PREBUILT_DIRS_RELEASE}/libdullahan.a
        ${APPKIT_LIBRARY}
       )

    execute_process(
        COMMAND lipo Chromium\ Embedded\ Framework.framework/Chromium\ Embedded\ Framework
            -thin ${CMAKE_OSX_ARCHITECTURES}
            -output Chromium\ Embedded\ Framework.framework/Chromium\ Embedded\ Framework
        COMMAND lipo Chromium\ Embedded\ Framework.framework/Libraries/libEGL.dylib
            -thin ${CMAKE_OSX_ARCHITECTURES}
            -output Chromium\ Embedded\ Framework.framework/Libraries/libEGL.dylib
        COMMAND lipo Chromium\ Embedded\ Framework.framework/Libraries/libGLESv2.dylib
            -thin ${CMAKE_OSX_ARCHITECTURES}
            -output Chromium\ Embedded\ Framework.framework/Libraries/libGLESv2.dylib
        COMMAND lipo Chromium\ Embedded\ Framework.framework/Libraries/libcef_sandbox.dylib
            -thin ${CMAKE_OSX_ARCHITECTURES}
            -output Chromium\ Embedded\ Framework.framework/Libraries/libcef_sandbox.dylib
        COMMAND lipo Chromium\ Embedded\ Framework.framework/Libraries/libvk_swiftshader.dylib
            -thin ${CMAKE_OSX_ARCHITECTURES}
            -output Chromium\ Embedded\ Framework.framework/Libraries/libvk_swiftshader.dylib
        COMMAND lipo DullahanHelper\ \(Alerts\).app/Contents/MacOS/DullahanHelper\ \(Alerts\)
            -thin ${CMAKE_OSX_ARCHITECTURES}
            -output DullahanHelper\ \(Alerts\).app/Contents/MacOS/DullahanHelper\ \(Alerts\)
        COMMAND lipo DullahanHelper\ \(GPU\).app/Contents/MacOS/DullahanHelper\ \(GPU\)
            -thin ${CMAKE_OSX_ARCHITECTURES}
            -output DullahanHelper\ \(GPU\).app/Contents/MacOS/DullahanHelper\ \(GPU\)
        COMMAND lipo DullahanHelper\ \(Plugin\).app/Contents/MacOS/DullahanHelper\ \(Plugin\)
            -thin ${CMAKE_OSX_ARCHITECTURES}
            -output DullahanHelper\ \(Plugin\).app/Contents/MacOS/DullahanHelper\ \(Plugin\)
        COMMAND lipo DullahanHelper\ \(Renderer\).app/Contents/MacOS/DullahanHelper\ \(Renderer\)
            -thin ${CMAKE_OSX_ARCHITECTURES}
            -output DullahanHelper\ \(Renderer\).app/Contents/MacOS/DullahanHelper\ \(Renderer\)
        COMMAND lipo DullahanHelper.app/Contents/MacOS/DullahanHelper
            -thin ${CMAKE_OSX_ARCHITECTURES}
            -output DullahanHelper.app/Contents/MacOS/DullahanHelper
        COMMAND lipo libcef_dll_wrapper.a
            -thin ${CMAKE_OSX_ARCHITECTURES}
            -output libcef_dll_wrapper.a
        COMMAND lipo libdullahan.a
            -thin ${CMAKE_OSX_ARCHITECTURES}
            -output libdullahan.a
        WORKING_DIRECTORY ${ARCH_PREBUILT_DIRS_RELEASE}
    )

elseif (LINUX)
    target_link_libraries( ll::cef INTERFACE
        libdullahan.a
        cef
        cef_dll_wrapper.a
    )
endif (WINDOWS)
