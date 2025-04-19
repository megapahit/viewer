# -*- cmake -*-

include(Linking)
include(Prebuilt)

include_guard()
add_library( ll::meshoptimizer INTERFACE IMPORTED )

if (NOT USESYSTEMLIBS)
use_system_binary(meshoptimizer)
elseif (${LINUX_DISTRO} MATCHES debian OR (${LINUX_DISTRO} MATCHES ubuntu) OR CMAKE_SYSTEM_NAME MATCHES FreeBSD)
  find_package(meshoptimizer)
  target_link_libraries( ll::meshoptimizer INTERFACE meshoptimizer)
  return ()
endif (NOT USESYSTEMLIBS)

if (LINUX AND CMAKE_SYSTEM_PROCESSOR MATCHES x86_64 AND NOT (${LINUX_DISTRO} MATCHES gentoo) OR NOT USESYSTEMLIBS)
use_prebuilt_binary(meshoptimizer)
elseif (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/meshoptimizer_installed OR NOT ${meshoptimizer_installed} EQUAL 0)
  if (NOT EXISTS ${CMAKE_BINARY_DIR}/meshoptimizer-0.21.tar.gz)
    file(DOWNLOAD
      https://github.com/zeux/meshoptimizer/archive/refs/tags/v0.21.tar.gz
      ${CMAKE_BINARY_DIR}/meshoptimizer-0.21.tar.gz
      )
  endif (NOT EXISTS ${CMAKE_BINARY_DIR}/meshoptimizer-0.21.tar.gz)
  file(ARCHIVE_EXTRACT
    INPUT ${CMAKE_BINARY_DIR}/meshoptimizer-0.21.tar.gz
    DESTINATION ${CMAKE_BINARY_DIR}
    )
  if (DARWIN)
    try_compile(MESHOPTIMIZER_RESULT
      PROJECT meshoptimizer
      SOURCE_DIR ${CMAKE_BINARY_DIR}/meshoptimizer-0.21
      BINARY_DIR ${CMAKE_BINARY_DIR}/meshoptimizer-0.21
      TARGET meshoptimizer
      CMAKE_FLAGS
        -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=${CMAKE_OSX_DEPLOYMENT_TARGET}
      OUTPUT_VARIABLE meshoptimizer_installed
    )
  else ()
    try_compile(MESHOPTIMIZER_RESULT
      PROJECT meshoptimizer
      SOURCE_DIR ${CMAKE_BINARY_DIR}/meshoptimizer-0.21
      BINARY_DIR ${CMAKE_BINARY_DIR}/meshoptimizer-0.21
      TARGET meshoptimizer
      OUTPUT_VARIABLE meshoptimizer_installed
    )
  endif (DARWIN)
  if (${MESHOPTIMIZER_RESULT})
    file(
      COPY ${CMAKE_BINARY_DIR}/meshoptimizer-0.21/src/meshoptimizer.h
      DESTINATION ${LIBS_PREBUILT_DIR}/include/meshoptimizer
      )
    file(
      COPY ${CMAKE_BINARY_DIR}/meshoptimizer-0.21/libmeshoptimizer.a
      DESTINATION ${LIBS_PREBUILT_DIR}/lib/release
      )
    file(WRITE ${PREBUILD_TRACKING_DIR}/meshoptimizer_installed "${meshoptimizer_installed}")
  endif (${MESHOPTIMIZER_RESULT})
endif (LINUX AND CMAKE_SYSTEM_PROCESSOR MATCHES x86_64 AND NOT (${LINUX_DISTRO} MATCHES gentoo) OR NOT USESYSTEMLIBS)

if (NOT USESYSTEMLIBS AND WINDOWS)
  target_link_libraries( ll::meshoptimizer INTERFACE meshoptimizer.lib)
else (NOT USESYSTEMLIBS AND WINDOWS)
  target_link_libraries( ll::meshoptimizer INTERFACE libmeshoptimizer.a)
endif (NOT USESYSTEMLIBS AND WINDOWS)

target_include_directories( ll::meshoptimizer SYSTEM INTERFACE ${LIBS_PREBUILT_DIR}/include/meshoptimizer)
