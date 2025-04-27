# -*- cmake -*-
include(Prebuilt)
include(Linking)

include_guard()
add_library( ll::openssl INTERFACE IMPORTED )

if (NOT USESYSTEMLIBS)
use_system_binary(openssl)
endif (NOT USESYSTEMLIBS)
if (LINUX AND CMAKE_SYSTEM_PROCESSOR MATCHES x86_64 OR DARWIN OR NOT USESYSTEMLIBS)
use_prebuilt_binary(openssl)
  if (DARWIN)
    execute_process(
      COMMAND lipo -archs libcrypto.a
      WORKING_DIRECTORY ${ARCH_PREBUILT_DIRS_RELEASE}
      OUTPUT_VARIABLE crypto_archs
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
    if (NOT ${crypto_archs} STREQUAL ${CMAKE_OSX_ARCHITECTURES})
      execute_process(
        COMMAND lipo
          libcrypto.a
          -thin ${CMAKE_OSX_ARCHITECTURES}
          -output libcrypto.a
        WORKING_DIRECTORY ${ARCH_PREBUILT_DIRS_RELEASE}
        )
    endif (NOT ${crypto_archs} STREQUAL ${CMAKE_OSX_ARCHITECTURES})
    execute_process(
      COMMAND lipo -archs libssl.a
      WORKING_DIRECTORY ${ARCH_PREBUILT_DIRS_RELEASE}
      OUTPUT_VARIABLE ssl_archs
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
    if (NOT ${ssl_archs} STREQUAL ${CMAKE_OSX_ARCHITECTURES})
      execute_process(
        COMMAND lipo
          libssl.a
          -thin ${CMAKE_OSX_ARCHITECTURES}
          -output libssl.a
        WORKING_DIRECTORY ${ARCH_PREBUILT_DIRS_RELEASE}
        )
    endif (NOT ${ssl_archs} STREQUAL ${CMAKE_OSX_ARCHITECTURES})
  endif (DARWIN)
elseif (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/openssl_installed OR NOT ${openssl_installed} EQUAL 0)
  if (NOT EXISTS ${CMAKE_BINARY_DIR}/OpenSSL_1_1_1w.tar.gz)
    file(DOWNLOAD
      https://github.com/openssl/openssl/archive/refs/tags/OpenSSL_1_1_1w.tar.gz
      ${CMAKE_BINARY_DIR}/OpenSSL_1_1_1w.tar.gz
      )
  endif ()
  file(ARCHIVE_EXTRACT
    INPUT ${CMAKE_BINARY_DIR}/OpenSSL_1_1_1w.tar.gz
    DESTINATION ${CMAKE_BINARY_DIR}
    )
  execute_process(
    COMMAND ./config no-shared --openssldir=${LIBS_PREBUILT_DIR}/ssl --prefix=${LIBS_PREBUILT_DIR} --libdir=${ARCH_PREBUILT_DIRS_RELEASE}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w
    )
  execute_process(
    COMMAND make -j${MAKE_JOBS}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w
    )
  execute_process(
    COMMAND make install
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w
    RESULT_VARIABLE openssl_installed
    )
  file(WRITE ${PREBUILD_TRACKING_DIR}/openssl_installed "${openssl_installed}")
endif (LINUX AND CMAKE_SYSTEM_PROCESSOR MATCHES x86_64 OR DARWIN OR NOT USESYSTEMLIBS)
if (WINDOWS AND NOT USESYSTEMLIBS)
  target_link_libraries(ll::openssl INTERFACE ${ARCH_PREBUILT_DIRS_RELEASE}/libssl.lib ${ARCH_PREBUILT_DIRS_RELEASE}/libcrypto.lib Crypt32.lib)
elseif (LINUX)
  target_link_libraries(ll::openssl INTERFACE ${ARCH_PREBUILT_DIRS_RELEASE}/libssl.a ${ARCH_PREBUILT_DIRS_RELEASE}/libcrypto.a dl)
else()
  target_link_libraries(ll::openssl INTERFACE ssl crypto)
endif (WINDOWS AND NOT USESYSTEMLIBS)
if (NOT (WINDOWS AND USESYSTEMLIBS))
target_include_directories( ll::openssl SYSTEM INTERFACE ${LIBS_PREBUILT_DIR}/include)
endif (NOT (WINDOWS AND USESYSTEMLIBS))

