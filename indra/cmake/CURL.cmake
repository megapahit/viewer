# -*- cmake -*-
include(Prebuilt)
include(Linking)

include_guard()
add_library( ll::libcurl INTERFACE IMPORTED )

if (NOT USESYSTEMLIBS)
use_system_binary(libcurl)
endif (NOT USESYSTEMLIBS)
if (DARWIN OR LINUX OR NOT USESYSTEMLIBS)
use_prebuilt_binary(curl)
  if (DARWIN)
    execute_process(
      COMMAND lipo -archs libcurl.a
      WORKING_DIRECTORY ${LIBS_PREBUILT_DIR}/lib/release
      OUTPUT_VARIABLE curl_archs
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
    if (NOT ${curl_archs} STREQUAL ${CMAKE_OSX_ARCHITECTURES})
      execute_process(
        COMMAND lipo
          libcurl.a
          -thin ${CMAKE_OSX_ARCHITECTURES}
          -output libcurl.a
        WORKING_DIRECTORY ${LIBS_PREBUILT_DIR}/lib/release
        )
    endif (NOT ${curl_archs} STREQUAL ${CMAKE_OSX_ARCHITECTURES})
  endif (DARWIN)
elseif (CMAKE_SYSTEM_NAME MATCHES FreeBSD AND (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/curl_installed OR NOT ${curl_installed} EQUAL 0))
  if (NOT EXISTS ${CMAKE_BINARY_DIR}/3p-curl-7.54.1-r1.tar.gz)
    file(DOWNLOAD
      https://github.com/secondlife/3p-curl/archive/refs/tags/v7.54.1-r1.tar.gz
      ${CMAKE_BINARY_DIR}/3p-curl-7.54.1-r1.tar.gz
      )
  endif (NOT EXISTS ${CMAKE_BINARY_DIR}/3p-curl-7.54.1-r1.tar.gz)
  file(ARCHIVE_EXTRACT
    INPUT ${CMAKE_BINARY_DIR}/3p-curl-7.54.1-r1.tar.gz
    DESTINATION ${CMAKE_BINARY_DIR}
    )
  file(
    COPY
      ${CMAKE_BINARY_DIR}/3p-curl-7.54.1-r1/curl/include/curl/curl.h
      ${CMAKE_BINARY_DIR}/3p-curl-7.54.1-r1/curl/include/curl/curlbuild.h
      ${CMAKE_BINARY_DIR}/3p-curl-7.54.1-r1/curl/include/curl/curlrules.h
      ${CMAKE_BINARY_DIR}/3p-curl-7.54.1-r1/curl/include/curl/curlver.h
      ${CMAKE_BINARY_DIR}/3p-curl-7.54.1-r1/curl/include/curl/easy.h
      ${CMAKE_BINARY_DIR}/3p-curl-7.54.1-r1/curl/include/curl/mprintf.h
      ${CMAKE_BINARY_DIR}/3p-curl-7.54.1-r1/curl/include/curl/multi.h
      ${CMAKE_BINARY_DIR}/3p-curl-7.54.1-r1/curl/include/curl/stdcheaders.h
      ${CMAKE_BINARY_DIR}/3p-curl-7.54.1-r1/curl/include/curl/system.h
      ${CMAKE_BINARY_DIR}/3p-curl-7.54.1-r1/curl/include/curl/typecheck-gcc.h
    DESTINATION ${LIBS_PREBUILT_DIR}/include/curl
    )
  file(
    COPY
      ${LIBS_PREBUILT_DIR}/lib/release/libcrypto.a
      ${LIBS_PREBUILT_DIR}/lib/release/libssl.a
    DESTINATION ${LIBS_PREBUILT_DIR}/lib
    )
  message("We need to temporarily have OpenSSL3 header directory and libraries renamed just until the libcurl building process with OpenSSL1.1 now is finished.")
  execute_process(COMMAND sudo mv /usr/include/openssl /usr/include/openssl3)
  execute_process(COMMAND sudo mv /usr/lib/libcrypto.a /usr/lib/libcrypto.a.3)
  execute_process(COMMAND sudo mv /usr/lib/libcrypto.so /usr/lib/libcrypto.so.3)
  execute_process(COMMAND sudo mv /usr/lib/libssl.a /usr/lib/libssl.a.3)
  execute_process(COMMAND sudo mv /usr/lib/libssl.so /usr/lib/libssl.so.3)
  execute_process(
    COMMAND ./configure --disable-alt-svc --disable-dict --disable-doh --disable-file --disable-gopher --disable-headers-api --disable-hsts --disable-imap --disable-ldap --disable-ldaps --disable-libcurl-option --disable-manual --disable-mqtt --disable-ntlm --disable-ntlm-wb --disable-pop3 --disable-rtsp --disable-shared --disable-smb --disable-smtp --disable-sspi --disable-telnet --disable-tftp --disable-tls-srp --disable-unix-sockets --disable-verbose --disable-versioned-symbols --enable-threaded-resolver --with-ssl=${LIBS_PREBUILT_DIR} --without-libidn2 --without-libpsl --without-libssh2
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/3p-curl-7.54.1-r1/curl
    )
  execute_process(
    COMMAND make -j${MAKE_JOBS}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/3p-curl-7.54.1-r1/curl
    RESULT_VARIABLE curl_installed
    )
  file(
    COPY ${CMAKE_BINARY_DIR}/3p-curl-7.54.1-r1/curl/lib/.libs/libcurl.a
    DESTINATION ${LIBS_PREBUILT_DIR}/lib/release
    )
  execute_process(COMMAND sudo mv /usr/include/openssl3 /usr/include/openssl)
  execute_process(COMMAND sudo mv /usr/lib/libcrypto.a.3 /usr/lib/libcrypto.a)
  execute_process(COMMAND sudo mv /usr/lib/libcrypto.so.3 /usr/lib/libcrypto.so)
  execute_process(COMMAND sudo mv /usr/lib/libssl.a.3 /usr/lib/libssl.a)
  execute_process(COMMAND sudo mv /usr/lib/libssl.so.3 /usr/lib/libssl.so)
  message("OpenSSL3 header directory and library names have been restored.")
  file(REMOVE
    ${LIBS_PREBUILT_DIR}/lib/libcrypto.a
    ${LIBS_PREBUILT_DIR}/lib/libssl.a
    )
  file(WRITE ${PREBUILD_TRACKING_DIR}/curl_installed "${curl_installed}")
endif (DARWIN OR LINUX OR NOT USESYSTEMLIBS)
if (WINDOWS AND NOT USESYSTEMLIBS)
  target_link_libraries(ll::libcurl INTERFACE
    ${ARCH_PREBUILT_DIRS_RELEASE}/libcurl.lib
    ll::openssl
    ll::nghttp2
    ll::zlib-ng
    )
else ()
  target_link_libraries(ll::libcurl INTERFACE
    ${ARCH_PREBUILT_DIRS_RELEASE}/libcurl.a
    ll::openssl
    ll::nghttp2
    ll::zlib-ng
    )
endif ()
target_include_directories( ll::libcurl SYSTEM INTERFACE ${LIBS_PREBUILT_DIR}/include)
