# -*- cmake -*-
include(Prebuilt)

set(NDOF ON CACHE BOOL "Use NDOF space navigator joystick library.")

include_guard()
add_library( ll::ndof INTERFACE IMPORTED )

if (NDOF)
  if (WINDOWS OR DARWIN)
    if (NOT USESYSTEMLIBS)
    use_prebuilt_binary(libndofdev)
    elseif (DARWIN AND (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/libndofdev_installed OR NOT ${libndofdev_installed} EQUAL 0))
      file(DOWNLOAD
        https://github.com/secondlife/3p-libndofdev/archive/refs/tags/v0.1.8e9edc7.tar.gz
        ${CMAKE_BINARY_DIR}/3p-libndofdev-0.1.8e9edc7.tar.gz
        )
      file(ARCHIVE_EXTRACT
        INPUT ${CMAKE_BINARY_DIR}/3p-libndofdev-0.1.8e9edc7.tar.gz
        DESTINATION ${CMAKE_BINARY_DIR}
        )
      try_compile(LIBNDOFDEV_RESULT
        PROJECT libndofdev
        SOURCE_DIR ${CMAKE_BINARY_DIR}/3p-libndofdev-0.1.8e9edc7/libndofdev
        BINARY_DIR ${CMAKE_BINARY_DIR}/3p-libndofdev-0.1.8e9edc7/libndofdev
        TARGET ndofdev
        CMAKE_FLAGS
          -DCMAKE_C_FLAGS:STRING=-DTARGET_OS_MAC\ -Wno-int-conversion
          -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=${CMAKE_OSX_DEPLOYMENT_TARGET}
        OUTPUT_VARIABLE libndofdev_installed
        )
      if (${LIBNDOFDEV_RESULT})
        file(
          COPY ${CMAKE_BINARY_DIR}/3p-libndofdev-0.1.8e9edc7/libndofdev/src/ndofdev_external.h
          DESTINATION ${LIBS_PREBUILT_DIR}/include
          )
        file(
          COPY ${CMAKE_BINARY_DIR}/3p-libndofdev-0.1.8e9edc7/libndofdev/src/libndofdev.dylib
          DESTINATION ${LIBS_PREBUILT_DIR}/lib/release
          )
        file(WRITE ${PREBUILD_TRACKING_DIR}/libndofdev_installed "${libndofdev_installed}")
      endif (${LIBNDOFDEV_RESULT})
    endif (NOT USESYSTEMLIBS)
  elseif (LINUX)
    use_prebuilt_binary(open-libndofdev)
  endif (WINDOWS OR DARWIN)

  if (WINDOWS)
    target_link_libraries( ll::ndof INTERFACE libndofdev)
  elseif (DARWIN OR LINUX)
    target_link_libraries( ll::ndof INTERFACE ndofdev)
  endif (WINDOWS)
  target_compile_definitions( ll::ndof INTERFACE LIB_NDOF=1)
else()
  add_compile_options(-ULIB_NDOF)
endif (NDOF)
