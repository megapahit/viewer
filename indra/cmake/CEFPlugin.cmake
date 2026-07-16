# -*- cmake -*-
include(Linking)
include(Prebuilt)

include_guard()
add_library( ll::cef INTERFACE IMPORTED )

if (${LINUX_DISTRO} MATCHES arch)
    if (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/dullahan_installed OR NOT ${dullahan_installed} EQUAL 0)
        file(
            COPY /usr/src/cef/libcef_dll
            DESTINATION ${CMAKE_BINARY_DIR}
        )
        execute_process(
            COMMAND sed -i "s/macro(L/cmake_minimum_required(VERSION 3.28)\\nmacro(SET_LIBRARY_TARGET_PROPERTIES)\\nendmacro()\\nmacro(L/" CMakeLists.txt
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/libcef_dll
        )
        try_compile(LIBCEF_DLL_RESULT
            PROJECT libcef_dll
            SOURCE_DIR ${CMAKE_BINARY_DIR}/libcef_dll
            BINARY_DIR ${CMAKE_BINARY_DIR}/libcef_dll
            CMAKE_FLAGS
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                "-DCMAKE_CXX_FLAGS:STRING=-I/usr/include/cef -I/usr/src/cef -fPIC"
        )
        if (${LIBCEF_DLL_RESULT})
            file(
                COPY ${CMAKE_BINARY_DIR}/libcef_dll/libcef_dll_wrapper.a
                DESTINATION ${ARCH_PREBUILT_DIRS_RELEASE}
            )
        endif ()
        if (NOT EXISTS ${CMAKE_BINARY_DIR}/dullahan-1.31.0-CEF_148.0.9.tar.gz)
            file(DOWNLOAD
                https://github.com/secondlife/dullahan/archive/refs/tags/v1.31.0-CEF_148.0.9.tar.gz
                ${CMAKE_BINARY_DIR}/dullahan-1.31.0-CEF_148.0.9.tar.gz
            )
        endif ()
        file(ARCHIVE_EXTRACT
            INPUT ${CMAKE_BINARY_DIR}/dullahan-1.31.0-CEF_148.0.9.tar.gz
            DESTINATION ${CMAKE_BINARY_DIR}
        )
        try_compile(DULLAHAN_RESULT
            PROJECT dullahan
            SOURCE_DIR ${CMAKE_BINARY_DIR}/dullahan-1.31.0-CEF_148.0.9
            BINARY_DIR ${CMAKE_BINARY_DIR}/dullahan-1.31.0-CEF_148.0.9
            CMAKE_FLAGS
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                -DCMAKE_INSTALL_PREFIX:PATH=${LIBS_PREBUILT_DIR}
                -DCMAKE_INSTALL_LIBDIR:PATH=${ARCH_PREBUILT_DIRS_RELEASE}
                -DCEF_WRAPPER_DIR:PATH=/usr/include/cef
                -DCEF_WRAPPER_BUILD_DIR:PATH=${CMAKE_BINARY_DIR}/dullahan-1.31.0-CEF_148.0.9
                -DCEF_LIBRARY_RELEASE:FILEPATH=${INSTALL_PREFIX}/lib/cef/libcef.so
                -DCEF_DLL_LIBRARY_RELEASE:FILEPATH=${ARCH_PREBUILT_DIRS_RELEASE}/libcef_dll_wrapper.a
                "-DCMAKE_CXX_FLAGS:STRING=-I/usr/include/cef -I/usr/src/cef -DWRAPPING_CEF_SHARED"
        )
        if (${DULLAHAN_RESULT})
            file(MAKE_DIRECTORY ${LIBS_PREBUILT_DIR}/bin/release)
            file(
                COPY ${CMAKE_BINARY_DIR}/dullahan-1.31.0-CEF_148.0.9/dullahan_host
                DESTINATION ${LIBS_PREBUILT_DIR}/bin/release
            )
            if (CMAKE_BUILD_TYPE MATCHES Release)
                execute_process(
                    COMMAND ${CMAKE_STRIP} dullahan_host
                    WORKING_DIRECTORY ${LIBS_PREBUILT_DIR}/bin/release
                )
            endif ()
            file(
                COPY ${CMAKE_BINARY_DIR}/dullahan-1.31.0-CEF_148.0.9/libdullahan.a
                DESTINATION ${ARCH_PREBUILT_DIRS_RELEASE}
            )
            file(MAKE_DIRECTORY ${LIBS_PREBUILT_DIR}/include/cef)
            file(
                COPY
                    ${CMAKE_BINARY_DIR}/dullahan-1.31.0-CEF_148.0.9/src/dullahan.h
                    ${CMAKE_BINARY_DIR}/dullahan-1.31.0-CEF_148.0.9/src/dullahan_version.h
                DESTINATION ${LIBS_PREBUILT_DIR}/include/cef
            )
            file(WRITE ${PREBUILD_TRACKING_DIR}/dullahan_installed "0")
        endif ()
    endif ()
elseif (${LINUX_DISTRO} MATCHES fedora)
    if (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/dullahan_installed OR NOT ${dullahan_installed} EQUAL 0)
        file(
            COPY /usr/src/cef-146.0.11/libcef_dll
            DESTINATION ${CMAKE_BINARY_DIR}
        )
        execute_process(
            COMMAND sed -i "s/macro(L/cmake_minimum_required(VERSION 3.28)\\nmacro(SET_LIBRARY_TARGET_PROPERTIES)\\nendmacro()\\nmacro(L/" CMakeLists.txt
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/libcef_dll
        )
        try_compile(LIBCEF_DLL_RESULT
            PROJECT libcef_dll
            SOURCE_DIR ${CMAKE_BINARY_DIR}/libcef_dll
            BINARY_DIR ${CMAKE_BINARY_DIR}/libcef_dll
            CMAKE_FLAGS
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                "-DCMAKE_CXX_FLAGS:STRING=-I/usr/include/cef -I/usr/src/cef-146.0.11 -fPIC"
        )
        if (${LIBCEF_DLL_RESULT})
            file(
                COPY ${CMAKE_BINARY_DIR}/libcef_dll/libcef_dll_wrapper.a
                DESTINATION ${ARCH_PREBUILT_DIRS_RELEASE}
            )
        endif ()
        if (NOT EXISTS ${CMAKE_BINARY_DIR}/dullahan-1.29.0-CEF_146.0.12.tar.gz)
            file(DOWNLOAD
                https://github.com/secondlife/dullahan/archive/refs/tags/v1.29.0-CEF_146.0.12.tar.gz
                ${CMAKE_BINARY_DIR}/dullahan-1.29.0-CEF_146.0.12.tar.gz
            )
        endif ()
        file(ARCHIVE_EXTRACT
            INPUT ${CMAKE_BINARY_DIR}/dullahan-1.29.0-CEF_146.0.12.tar.gz
            DESTINATION ${CMAKE_BINARY_DIR}
        )
        try_compile(DULLAHAN_RESULT
            PROJECT dullahan
            SOURCE_DIR ${CMAKE_BINARY_DIR}/dullahan-1.29.0-CEF_146.0.12
            BINARY_DIR ${CMAKE_BINARY_DIR}/dullahan-1.29.0-CEF_146.0.12
            CMAKE_FLAGS
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                -DCMAKE_INSTALL_PREFIX:PATH=${LIBS_PREBUILT_DIR}
                -DCMAKE_INSTALL_LIBDIR:PATH=${ARCH_PREBUILT_DIRS_RELEASE}
                -DCEF_WRAPPER_DIR:PATH=/usr/include/cef
                -DCEF_WRAPPER_BUILD_DIR:PATH=${CMAKE_BINARY_DIR}/dullahan-1.29.0-CEF_146.0.12
                -DCEF_LIBRARY_RELEASE:FILEPATH=${INSTALL_PREFIX}/lib${ADDRESS_SIZE}/cef/libcef.so
                -DCEF_DLL_LIBRARY_RELEASE:FILEPATH=${ARCH_PREBUILT_DIRS_RELEASE}/libcef_dll_wrapper.a
                "-DCMAKE_CXX_FLAGS:STRING=-I/usr/include/cef -I/usr/src/cef-146.0.11 -DWRAPPING_CEF_SHARED"
        )
        if (${DULLAHAN_RESULT})
            file(MAKE_DIRECTORY ${LIBS_PREBUILT_DIR}/bin/release)
            file(
                COPY ${CMAKE_BINARY_DIR}/dullahan-1.29.0-CEF_146.0.12/dullahan_host
                DESTINATION ${LIBS_PREBUILT_DIR}/bin/release
            )
            if (CMAKE_BUILD_TYPE MATCHES Release)
                execute_process(
                    COMMAND ${CMAKE_STRIP} dullahan_host
                    WORKING_DIRECTORY ${LIBS_PREBUILT_DIR}/bin/release
                )
            endif ()
            file(
                COPY ${CMAKE_BINARY_DIR}/dullahan-1.29.0-CEF_146.0.12/libdullahan.a
                DESTINATION ${ARCH_PREBUILT_DIRS_RELEASE}
            )
            file(MAKE_DIRECTORY ${LIBS_PREBUILT_DIR}/include/cef)
            file(
                COPY
                    ${CMAKE_BINARY_DIR}/dullahan-1.29.0-CEF_146.0.12/src/dullahan.h
                    ${CMAKE_BINARY_DIR}/dullahan-1.29.0-CEF_146.0.12/src/dullahan_version.h
                DESTINATION ${LIBS_PREBUILT_DIR}/include/cef
            )
            file(WRITE ${PREBUILD_TRACKING_DIR}/dullahan_installed "0")
        endif ()
    endif ()
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES aarch64)
    if (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/dullahan_installed OR NOT ${dullahan_installed} EQUAL 0)
        if (NOT EXISTS ${CMAKE_BINARY_DIR}/dullahan-1.24.0-CEF_139.0.40.tar.gz)
            file(DOWNLOAD
                https://github.com/secondlife/dullahan/archive/refs/tags/v1.24.0-CEF_139.0.40.tar.gz
                ${CMAKE_BINARY_DIR}/dullahan-1.24.0-CEF_139.0.40.tar.gz
                )
        endif ()
        file(ARCHIVE_EXTRACT
            INPUT ${CMAKE_BINARY_DIR}/dullahan-1.24.0-CEF_139.0.40.tar.gz
            DESTINATION ${CMAKE_BINARY_DIR}
            )
        execute_process(
            COMMAND sed -i "/#include <vector>/a #include <cstdint>" dullahan.h
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/dullahan-1.24.0-CEF_139.0.40/src
            )
        file(MAKE_DIRECTORY ${LIBS_PREBUILT_DIR}/include/cef)
        try_compile(DULLAHAN_RESULT
            PROJECT dullahan
            SOURCE_DIR ${CMAKE_BINARY_DIR}/dullahan-1.24.0-CEF_139.0.40
            BINARY_DIR ${CMAKE_BINARY_DIR}/dullahan-1.24.0-CEF_139.0.40
            CMAKE_FLAGS
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                -DCMAKE_INSTALL_PREFIX:PATH=${LIBS_PREBUILT_DIR}
                -DCMAKE_INSTALL_LIBDIR:PATH=${ARCH_PREBUILT_DIRS_RELEASE}
                -DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=ON
                -DUSE_SPOTIFY_CEF:BOOL=ON
                -DSPOTIFY_CEF_URL:STRING=https://cef-builds.spotifycdn.com/cef_binary_139.0.40%2Bg465474a%2Bchromium-139.0.7258.139_linuxarm64_minimal.tar.bz2
                -DPROJECT_ARCH:STRING=${CMAKE_SYSTEM_PROCESSOR}
        )
        if (${DULLAHAN_RESULT})
            if (CMAKE_BUILD_TYPE MATCHES Release)
                execute_process(
                    COMMAND ${CMAKE_STRIP}
                        chrome-sandbox
                        libEGL.so
                        libGLESv2.so
                        libcef.so
                        libvk_swiftshader.so
                        libvulkan.so.1
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/dullahan-1.24.0-CEF_139.0.40/_deps/cef_prebuild-src/${CMAKE_BUILD_TYPE}
                )
            endif ()
            execute_process(
                COMMAND ${CMAKE_MAKE_PROGRAM} install
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/dullahan-1.24.0-CEF_139.0.40
                OUTPUT_VARIABLE dullahan_installed
            )
            if (CMAKE_BUILD_TYPE MATCHES Release)
                execute_process(
                    COMMAND ${CMAKE_STRIP} dullahan_host
                    WORKING_DIRECTORY ${LIBS_PREBUILT_DIR}/bin/release
                )
            endif ()
            file(
                COPY
                    ${CMAKE_BINARY_DIR}/dullahan-1.24.0-CEF_139.0.40/src/dullahan.h
                    ${CMAKE_BINARY_DIR}/dullahan-1.24.0-CEF_139.0.40/src/dullahan_version.h
                DESTINATION ${LIBS_PREBUILT_DIR}/include/cef
                )
            file(WRITE ${PREBUILD_TRACKING_DIR}/dullahan_installed "${dullahan_installed}")
        endif (${DULLAHAN_RESULT})
    endif (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/dullahan_installed OR NOT ${dullahan_installed} EQUAL 0)
else ()
use_prebuilt_binary(dullahan)
endif ()
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
    if (NOT USE_FLATPAK)
        execute_process(
            COMMAND patchelf --set-rpath ${INSTALL_LIBRARY_DIR} dullahan_host
            WORKING_DIRECTORY ${LIBS_PREBUILT_DIR}/bin/release
        )
    endif ()
    if (${LINUX_DISTRO} MATCHES arch OR (${LINUX_DISTRO} MATCHES fedora))
        target_include_directories( ll::cef SYSTEM INTERFACE /usr/include/cef/include)
        if (${LINUX_DISTRO} MATCHES fedora)
            set(LIB_SUFFIX ${ADDRESS_SIZE})
        endif ()
        target_link_directories( ll::cef INTERFACE ${INSTALL_PREFIX}/lib${LIB_SUFFIX}/cef )
        execute_process(
            COMMAND patchelf --add-rpath ${INSTALL_PREFIX}/lib${LIB_SUFFIX}/cef dullahan_host
            WORKING_DIRECTORY ${LIBS_PREBUILT_DIR}/bin/release
        )
    elseif (CMAKE_SYSTEM_PROCESSOR MATCHES x86_64 AND (CMAKE_BUILD_TYPE MATCHES Release))
        execute_process(
            COMMAND ${CMAKE_STRIP}
                bin/release/chrome-sandbox
                bin/release/dullahan_host
                lib/release/libEGL.so
                lib/release/libvk_swiftshader.so
            WORKING_DIRECTORY ${LIBS_PREBUILT_DIR}
        )
    endif ()
    target_link_libraries( ll::cef INTERFACE
        libdullahan.a
        cef
        cef_dll_wrapper.a
    )
endif (WINDOWS)
