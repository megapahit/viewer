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
      WORKING_DIRECTORY ${LIBS_PREBUILT_DIR}/lib/release
      OUTPUT_VARIABLE crypto_archs
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
    if (NOT ${crypto_archs} STREQUAL ${CMAKE_OSX_ARCHITECTURES})
      execute_process(
        COMMAND lipo
          libcrypto.a
          -thin ${CMAKE_OSX_ARCHITECTURES}
          -output libcrypto.a
        WORKING_DIRECTORY ${LIBS_PREBUILT_DIR}/lib/release
        )
    endif (NOT ${crypto_archs} STREQUAL ${CMAKE_OSX_ARCHITECTURES})
    execute_process(
      COMMAND lipo -archs libssl.a
      WORKING_DIRECTORY ${LIBS_PREBUILT_DIR}/lib/release
      OUTPUT_VARIABLE ssl_archs
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
    if (NOT ${ssl_archs} STREQUAL ${CMAKE_OSX_ARCHITECTURES})
      execute_process(
        COMMAND lipo
          libssl.a
          -thin ${CMAKE_OSX_ARCHITECTURES}
          -output libssl.a
        WORKING_DIRECTORY ${LIBS_PREBUILT_DIR}/lib/release
        )
    endif (NOT ${ssl_archs} STREQUAL ${CMAKE_OSX_ARCHITECTURES})
  endif (DARWIN)
elseif (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/openssl_installed OR NOT ${openssl_installed} EQUAL 0)
  if (NOT EXISTS ${CMAKE_BINARY_DIR}/OpenSSL_1_1_1w.tar.gz)
    file(DOWNLOAD
      https://github.com/openssl/openssl/archive/refs/tags/OpenSSL_1_1_1w.tar.gz
      ${CMAKE_BINARY_DIR}/OpenSSL_1_1_1w.tar.gz
      )
  endif (NOT EXISTS ${CMAKE_BINARY_DIR}/OpenSSL_1_1_1w.tar.gz)
  file(ARCHIVE_EXTRACT
    INPUT ${CMAKE_BINARY_DIR}/OpenSSL_1_1_1w.tar.gz
    DESTINATION ${CMAKE_BINARY_DIR}
    )
  execute_process(
    COMMAND ./config no-shared
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w
    )
  execute_process(
    COMMAND make -j${MAKE_JOBS}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w
    RESULT_VARIABLE openssl_installed
    )
  file(
    COPY
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/aes.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/asn1.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/asn1_mac.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/asn1err.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/asn1t.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/async.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/asyncerr.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/bio.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/bioerr.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/blowfish.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/bn.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/bnerr.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/buffer.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/buffererr.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/camellia.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/cast.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/cmac.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/cms.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/cmserr.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/comp.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/comperr.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/conf.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/conf_api.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/conferr.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/crypto.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/cryptoerr.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/ct.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/cterr.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/des.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/dh.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/dherr.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/dsa.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/dsaerr.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/dtls1.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/e_os2.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/ebcdic.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/ec.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/ecdh.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/ecdsa.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/ecerr.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/engine.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/engineerr.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/err.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/evp.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/evperr.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/hmac.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/idea.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/kdf.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/kdferr.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/lhash.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/md2.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/md4.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/md5.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/mdc2.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/modes.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/obj_mac.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/objects.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/objectserr.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/ocsp.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/ocsperr.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/opensslconf.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/opensslv.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/ossl_typ.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/pem.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/pem2.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/pemerr.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/pkcs12.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/pkcs12err.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/pkcs7.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/pkcs7err.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/rand.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/rand_drbg.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/randerr.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/rc2.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/rc4.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/rc5.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/ripemd.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/rsa.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/rsaerr.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/safestack.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/seed.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/sha.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/srp.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/srtp.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/ssl.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/ssl2.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/ssl3.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/sslerr.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/stack.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/store.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/storeerr.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/symhacks.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/tls1.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/ts.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/tserr.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/txt_db.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/ui.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/uierr.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/whrlpool.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/x509.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/x509_vfy.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/x509err.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/x509v3.h
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/include/openssl/x509v3err.h
    DESTINATION ${LIBS_PREBUILT_DIR}/include/openssl
    )
  file(
    COPY
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/libcrypto.a
      ${CMAKE_BINARY_DIR}/openssl-OpenSSL_1_1_1w/libssl.a
    DESTINATION ${LIBS_PREBUILT_DIR}/lib/release
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

