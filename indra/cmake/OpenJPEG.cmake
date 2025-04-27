# -*- cmake -*-
include(Prebuilt)

include_guard()
add_library( ll::openjpeg INTERFACE IMPORTED )

if (NOT USESYSTEMLIBS)
use_system_binary(openjpeg)
endif (NOT USESYSTEMLIBS)
if (LINUX AND CMAKE_SYSTEM_PROCESSOR MATCHES x86_64 OR NOT USESYSTEMLIBS)
use_prebuilt_binary(openjpeg)
elseif (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/openjpeg_installed OR NOT ${openjpeg_installed} EQUAL 0)
  if (NOT EXISTS ${CMAKE_BINARY_DIR}/3p-openjpeg-2.5.0.ea12248.tar.gz)
    file(DOWNLOAD
      https://github.com/secondlife/3p-openjpeg/archive/refs/tags/v2.5.0.ea12248.tar.gz
      ${CMAKE_BINARY_DIR}/3p-openjpeg-2.5.0.ea12248.tar.gz
      )
  endif (NOT EXISTS ${CMAKE_BINARY_DIR}/3p-openjpeg-2.5.0.ea12248.tar.gz)
  file(ARCHIVE_EXTRACT
    INPUT ${CMAKE_BINARY_DIR}/3p-openjpeg-2.5.0.ea12248.tar.gz
    DESTINATION ${CMAKE_BINARY_DIR}
    )
  try_compile(OPENJPEG_RESULT
    PROJECT OPENJPEG
    SOURCE_DIR ${CMAKE_BINARY_DIR}/3p-openjpeg-2.5.0.ea12248/openjpeg
    BINARY_DIR ${CMAKE_BINARY_DIR}/3p-openjpeg-2.5.0.ea12248/openjpeg
    TARGET openjp2
    CMAKE_FLAGS
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DCMAKE_OSX_ARCHITECTURES:STRING=${CMAKE_OSX_ARCHITECTURES}
      -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=${CMAKE_OSX_DEPLOYMENT_TARGET}
      -DCMAKE_INSTALL_PREFIX:PATH=${LIBS_PREBUILT_DIR}
      -DCMAKE_INSTALL_LIBDIR:PATH=${ARCH_PREBUILT_DIRS_RELEASE}
      -DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=ON
      -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
      -DBUILD_CODEC:BOOL=OFF
    )
  if (${OPENJPEG_RESULT})
    execute_process(
      COMMAND ${CMAKE_MAKE_PROGRAM} install
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/3p-openjpeg-2.5.0.ea12248/openjpeg
      OUTPUT_VARIABLE openjpeg_installed
      )
    file(RENAME
      ${LIBS_PREBUILT_DIR}/include/openjpeg-2.5
      ${LIBS_PREBUILT_DIR}/include/openjpeg
      )
    file(
      COPY
        ${CMAKE_BINARY_DIR}/3p-openjpeg-2.5.0.ea12248/openjpeg/src/lib/openjp2/cio.h
        ${CMAKE_BINARY_DIR}/3p-openjpeg-2.5.0.ea12248/openjpeg/src/lib/openjp2/event.h
        ${CMAKE_BINARY_DIR}/3p-openjpeg-2.5.0.ea12248/openjpeg/src/lib/openjp2/opj_config_private.h
      DESTINATION ${LIBS_PREBUILT_DIR}/include/openjpeg
      )
    file(WRITE ${PREBUILD_TRACKING_DIR}/openjpeg_installed "${openjpeg_installed}")
  endif (${OPENJPEG_RESULT})
endif (LINUX AND CMAKE_SYSTEM_PROCESSOR MATCHES x86_64 OR NOT USESYSTEMLIBS)

target_link_libraries(ll::openjpeg INTERFACE openjp2 )
target_include_directories( ll::openjpeg SYSTEM INTERFACE ${LIBS_PREBUILT_DIR}/include)
