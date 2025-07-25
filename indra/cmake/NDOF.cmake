# -*- cmake -*-
include(Prebuilt)

set(NDOF ON CACHE BOOL "Use NDOF space navigator joystick library.")

include_guard()
add_library( ll::ndof INTERFACE IMPORTED )

if (NDOF)
  if (WINDOWS)
    use_prebuilt_binary(libndofdev)
  elseif (DARWIN)
    if (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/libndofdev_installed OR NOT ${libndofdev_installed} EQUAL 0)
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
          -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
          -DCMAKE_OSX_ARCHITECTURES:STRING=${CMAKE_OSX_ARCHITECTURES}
          -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=${CMAKE_OSX_DEPLOYMENT_TARGET}
          -DCMAKE_C_FLAGS:STRING=-DTARGET_OS_MAC\ -Wno-int-conversion
        OUTPUT_VARIABLE libndofdev_installed
        )
      if (${LIBNDOFDEV_RESULT})
        file(
          COPY ${CMAKE_BINARY_DIR}/3p-libndofdev-0.1.8e9edc7/libndofdev/src/ndofdev_external.h
          DESTINATION ${LIBS_PREBUILT_DIR}/include
          )
        file(
          COPY ${CMAKE_BINARY_DIR}/3p-libndofdev-0.1.8e9edc7/libndofdev/src/libndofdev.dylib
          DESTINATION ${ARCH_PREBUILT_DIRS_RELEASE}
          )
        file(WRITE ${PREBUILD_TRACKING_DIR}/libndofdev_installed "${libndofdev_installed}")
      endif ()
    endif ()
  elseif (LINUX)
    if (CMAKE_SYSTEM_PROCESSOR MATCHES x86_64)
    use_prebuilt_binary(open-libndofdev)
    else ()
      if (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/libndofdev_installed OR NOT ${libndofdev_installed} EQUAL 0)
        file(DOWNLOAD
          https://github.com/janoc/libndofdev/archive/refs/tags/v0.14.tar.gz
          ${CMAKE_BINARY_DIR}/libndofdev-0.14.tar.gz
          )
        file(ARCHIVE_EXTRACT
          INPUT ${CMAKE_BINARY_DIR}/libndofdev-0.14.tar.gz
          DESTINATION ${CMAKE_BINARY_DIR}
          )
        set(ENV{USE_SDL2} 1)
        execute_process(
          COMMAND make -j${MAKE_JOBS}
          WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/libndofdev-0.14
          RESULT_VARIABLE libndofdev_installed
          )
        unset(ENV{USE_SDL2})
        file(
          COPY ${CMAKE_BINARY_DIR}/libndofdev-0.14/ndofdev_external.h
          DESTINATION ${LIBS_PREBUILT_DIR}/include
          )
        file(
          COPY ${CMAKE_BINARY_DIR}/libndofdev-0.14/libndofdev.a
          DESTINATION ${ARCH_PREBUILT_DIRS_RELEASE}
          )
        file(WRITE ${PREBUILD_TRACKING_DIR}/libndofdev_installed "${libndofdev_installed}")
      endif ()
    endif ()
  endif ()

  if (WINDOWS)
    target_link_libraries( ll::ndof INTERFACE libndofdev)
  elseif (DARWIN OR LINUX)
    target_link_libraries( ll::ndof INTERFACE ndofdev)
  endif (WINDOWS)
  target_compile_definitions( ll::ndof INTERFACE LIB_NDOF=1)
else()
  add_compile_options(-ULIB_NDOF)
endif (NDOF)
