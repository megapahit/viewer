# -*- cmake -*-
include(Prebuilt)

include_guard()
add_library( ll::openjpeg INTERFACE IMPORTED )

#use_system_binary(openjpeg)
#use_prebuilt_binary(openjpeg)

if (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/openjpeg_installed OR NOT ${openjpeg_installed} EQUAL 0)
  if (NOT EXISTS ${CMAKE_BINARY_DIR}/openjpeg-2.5.3.tar.gz)
    file(DOWNLOAD
      https://github.com/uclouvain/openjpeg/archive/refs/tags/v2.5.3.tar.gz
      ${CMAKE_BINARY_DIR}/openjpeg-2.5.3.tar.gz
      )
  endif ()
  file(ARCHIVE_EXTRACT
    INPUT ${CMAKE_BINARY_DIR}/openjpeg-2.5.3.tar.gz
    DESTINATION ${CMAKE_BINARY_DIR}
    )

  if (${LINUX_DISTRO} MATCHES debian OR (${LINUX_DISTRO} MATCHES ubuntu))
    try_compile(OPENJPEG_RESULT
      PROJECT OPENJPEG
      SOURCE_DIR ${CMAKE_BINARY_DIR}/openjpeg-2.5.3
      BINARY_DIR ${CMAKE_BINARY_DIR}/openjpeg-2.5.3
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
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/openjpeg-2.5.3
        OUTPUT_VARIABLE openjpeg_installed
        )
    endif ()

  else ()
    execute_process(
      COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE} -DCMAKE_OSX_ARCHITECTURES:STRING=${CMAKE_OSX_ARCHITECTURES} -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=${CMAKE_OSX_DEPLOYMENT_TARGET} -DCMAKE_INSTALL_PREFIX:PATH=${LIBS_PREBUILT_DIR} -DCMAKE_INSTALL_LIBDIR:PATH=${ARCH_PREBUILT_DIRS_RELEASE} -DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=ON -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS} -DBUILD_CODEC:BOOL=OFF
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/openjpeg-2.5.3
      OUTPUT_VARIABLE openjpeg_installed
      )
  endif ()

  file(
    COPY
      ${CMAKE_BINARY_DIR}/openjpeg-2.5.3/src/lib/openjp2/cio.h
      ${CMAKE_BINARY_DIR}/openjpeg-2.5.3/src/lib/openjp2/event.h
      ${CMAKE_BINARY_DIR}/openjpeg-2.5.3/src/lib/openjp2/opj_config_private.h
    DESTINATION ${LIBS_PREBUILT_DIR}/include/openjpeg-2.5
    )
  file(WRITE ${PREBUILD_TRACKING_DIR}/openjpeg_installed "${openjpeg_installed}")
endif ()

if (${LINUX_DISTRO} MATCHES debian OR (${LINUX_DISTRO} MATCHES ubuntu))
target_link_libraries(ll::openjpeg INTERFACE openjp2 )
else ()
    include(FindPkgConfig)
    pkg_check_modules(Openjpeg REQUIRED libopenjp2)
    target_include_directories(ll::openjpeg SYSTEM INTERFACE ${Openjpeg_INCLUDE_DIRS})
    target_link_directories(ll::openjpeg INTERFACE ${Openjpeg_LIBRARY_DIRS})
    target_link_libraries(ll::openjpeg INTERFACE ${Openjpeg_LIBRARIES})
endif ()
target_include_directories( ll::openjpeg SYSTEM INTERFACE ${LIBS_PREBUILT_DIR}/include)
