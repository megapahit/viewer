# -*- cmake -*-
include(Prebuilt)

add_library(ll::vhacd INTERFACE IMPORTED)

if (FALSE)
use_system_binary(vhacd)
use_prebuilt_binary(vhacd)

elseif (NOT (${LINUX_DISTRO} MATCHES fedora))
    if (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/vhacd_installed OR NOT ${vhacd_installed} EQUAL 0)
        if (NOT EXISTS ${CMAKE_BINARY_DIR}/v-hacd-4.1.0.tar.gz)
            file(DOWNLOAD
                https://github.com/kmammou/v-hacd/archive/refs/tags/v4.1.0.tar.gz
                ${CMAKE_BINARY_DIR}/v-hacd-4.1.0.tar.gz
                )
        endif ()
        file(ARCHIVE_EXTRACT
            INPUT ${CMAKE_BINARY_DIR}/v-hacd-4.1.0.tar.gz
            DESTINATION ${CMAKE_BINARY_DIR}
            )
        file(MAKE_DIRECTORY ${LIBS_PREBUILT_DIR}/include/vhacd)
        file(
            COPY ${CMAKE_BINARY_DIR}/v-hacd-4.1.0/include/VHACD.h
            DESTINATION ${LIBS_PREBUILT_DIR}/include/vhacd
            )
        file(WRITE ${PREBUILD_TRACKING_DIR}/vhacd_installed "0")
    endif ()
target_include_directories(ll::vhacd SYSTEM INTERFACE ${LIBS_PREBUILT_DIR}/include/vhacd/)
endif ()
