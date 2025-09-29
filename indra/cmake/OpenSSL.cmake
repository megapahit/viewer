# -*- cmake -*-
include(Prebuilt)
include(Linking)

include_guard()
add_library( ll::openssl INTERFACE IMPORTED )

if ($ENV{MSYSTEM_CARCH} MATCHES aarch64)
use_system_binary(openssl)
elseif (LINUX AND CMAKE_SYSTEM_PROCESSOR MATCHES x86_64 OR DARWIN OR WINDOWS)
use_prebuilt_binary(openssl)

find_library(SSL_LIBRARY
    NAMES
    libssl.lib
    libssl.a
    PATHS "${ARCH_PREBUILT_DIRS_RELEASE}" REQUIRED NO_DEFAULT_PATH)

find_library(CRYPTO_LIBRARY
    NAMES
    libcrypto.lib
    libcrypto.a
    PATHS "${ARCH_PREBUILT_DIRS_RELEASE}" REQUIRED NO_DEFAULT_PATH)

target_link_libraries(ll::openssl INTERFACE ${SSL_LIBRARY} ${CRYPTO_LIBRARY})

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
    endif ()
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
    endif ()
  endif ()
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
endif ()
if (WINDOWS)
  target_link_libraries(ll::openssl INTERFACE Crypt32.lib)
endif (WINDOWS)

target_include_directories(ll::openssl SYSTEM INTERFACE ${LIBS_PREBUILT_DIR}/include)

