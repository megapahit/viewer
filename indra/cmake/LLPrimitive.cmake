# -*- cmake -*-

# these should be moved to their own cmake file
include(Prebuilt)
include(Linking)
include(Boost)

include_guard()

add_library( ll::minizip-ng INTERFACE IMPORTED )
add_library( ll::libxml INTERFACE IMPORTED )
add_library( ll::colladadom INTERFACE IMPORTED )

# ND, needs fixup in collada conan pkg
if( USE_CONAN )
  target_include_directories( ll::colladadom SYSTEM INTERFACE
    "${CONAN_INCLUDE_DIRS_COLLADADOM}/collada-dom/"
    "${CONAN_INCLUDE_DIRS_COLLADADOM}/collada-dom/1.4/" )
endif()

if( USESYSTEMLIBS )
  if( WINDOWS )
    include(FindPkgConfig)
    pkg_check_modules(Colladadom REQUIRED collada-dom-141)
    target_compile_definitions( ll::colladadom INTERFACE COLLADA_DOM_SUPPORT141 )
    target_include_directories( ll::colladadom SYSTEM INTERFACE ${Colladadom_INCLUDE_DIRS} )
    target_link_directories( ll::colladadom INTERFACE ${Colladadom_LIBRARY_DIRS} )
    target_link_libraries( ll::colladadom INTERFACE ${Colladadom_LIBRARIES} )
    return ()
  elseif( LINUX OR CMAKE_SYSTEM_NAME MATCHES FreeBSD )
    # Build of the collada-dom for Linux and FreeBSD is done in
    # indra/llprimitive/CMakeLists.txt
    return ()
  else( WINDOWS )
    include(FindPkgConfig)
    pkg_check_modules(Minizip REQUIRED minizip)
    pkg_check_modules(Libxml2 REQUIRED libxml-2.0)
    target_link_libraries( ll::minizip-ng INTERFACE ${Minizip_LIBRARIES} )
    target_link_libraries( ll::libxml INTERFACE ${Libxml2_LIBRARIES} )
    if( ${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/colladadom_installed OR NOT ${colladadom_installed} EQUAL 0 )
      if( NOT EXISTS ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8.tar.gz )
        file(DOWNLOAD
          https://github.com/secondlife/3p-colladadom/archive/refs/tags/v2.3-r8.tar.gz
          ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8.tar.gz
          )
      endif( NOT EXISTS ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8.tar.gz )
      file(ARCHIVE_EXTRACT
        INPUT ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8.tar.gz
        DESTINATION ${CMAKE_BINARY_DIR}
        )
      file(MAKE_DIRECTORY ${LIBS_PREBUILT_DIR}/include/collada/1.4)
      if( DARWIN )
        try_compile(COLLADADOM_RESULT
          PROJECT colladadom
          SOURCE_DIR ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8
          BINARY_DIR ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8
          TARGET collada14dom
          CMAKE_FLAGS
            -DCMAKE_CXX_STANDARD:STRING=17
            -DCMAKE_CXX_FLAGS:STRING=-I${Minizip_INCLUDE_DIRS}
            "-DCMAKE_SHARED_LINKER_FLAGS:STRING=-L${Minizip_LIBRARY_DIRS} -L${Minizip_LIBRARY_DIRS}exec/boost/1.87/lib"
            -DCMAKE_OSX_ARCHITECTURES:STRING=${CMAKE_OSX_ARCHITECTURES}
            -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=${CMAKE_OSX_DEPLOYMENT_TARGET}
            -DZLIB_LIBRARIES:STRING=${Libxml2_LIBRARIES}
            -DBoost_FILESYSTEM_LIBRARY:STRING=boost_filesystem-mt
            -DBoost_SYSTEM_LIBRARY:STRING=boost_system-mt
            -DEXTRA_COMPILE_FLAGS:STRING=-I${Libxml2_INCLUDE_DIRS}
            -DBoost_CFLAGS:STRING=-I${Libxml2_LIBRARY_DIRS}exec/boost/1.87/include
            -DOPT_COLLADA14:BOOL=ON
            -DCOLLADA_DOM_INCLUDE_INSTALL_DIR:FILEPATH=${LIBS_PREBUILT_DIR}/include/collada
            -DCOLLADA_DOM_SOVERSION:STRING=0
            -DCOLLADA_DOM_VERSION:STRING=2.3-r8
          OUTPUT_VARIABLE colladadom_installed
          )
        if( ${COLLADADOM_RESULT} )
          file(
            COPY
              ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8/src/1.4/libcollada14dom.2.3-r8.dylib
              ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8/src/1.4/libcollada14dom.0.dylib
              ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8/src/1.4/libcollada14dom.dylib
            DESTINATION ${LIBS_PREBUILT_DIR}/lib/release
            FOLLOW_SYMLINK_CHAIN
            )
        endif( ${COLLADADOM_RESULT} )
      else( DARWIN )
        execute_process(
          COMMAND sed -i "" -e "s/SHARED/STATIC/g" 1.4/CMakeLists.txt
          WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8/src
          )
        try_compile(COLLADADOM_RESULT
          PROJECT colladadom
          SOURCE_DIR ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8
          BINARY_DIR ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8
          TARGET collada14dom
          CMAKE_FLAGS
            -DCMAKE_CXX_STANDARD:STRING=17
            -DCMAKE_CXX_FLAGS:STRING=-I${Minizip_INCLUDE_DIRS}
            -DCMAKE_SHARED_LINKER_FLAGS:STRING=-L${Minizip_LIBRARY_DIRS}
            -DZLIB_LIBRARIES:STRING=${Libxml2_LIBRARIES}
            -DBoost_FILESYSTEM_LIBRARY:STRING=boost_filesystem
            -DBoost_SYSTEM_LIBRARY:STRING=boost_system
            -DEXTRA_COMPILE_FLAGS:STRING=-I${Libxml2_INCLUDE_DIRS}
            -DOPT_COLLADA14:BOOL=ON
            -DCOLLADA_DOM_INCLUDE_INSTALL_DIR:FILEPATH=${LIBS_PREBUILT_DIR}/include/collada
            -DCOLLADA_DOM_SOVERSION:STRING=0
            -DCOLLADA_DOM_VERSION:STRING=2.3-r8
          OUTPUT_VARIABLE colladadom_installed
          )
        if( ${COLLADADOM_RESULT} )
          file(
            COPY ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8/src/1.4/libcollada14dom.a
            DESTINATION ${LIBS_PREBUILT_DIR}/lib/release
            )
        endif( ${COLLADADOM_RESULT} )
      endif( DARWIN )
      if( ${COLLADADOM_RESULT} )
        file(REMOVE_RECURSE ${LIBS_PREBUILT_DIR}/include/collada/1.4)
        file(
          COPY
            ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8/include/1.4
            ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8/include/1.5
            ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8/include/dae
            ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8/include/dae.h
            ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8/include/dom.h
            ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8/include/modules
          DESTINATION ${LIBS_PREBUILT_DIR}/include/collada
          )
        file(WRITE ${PREBUILD_TRACKING_DIR}/colladadom_installed "${colladadom_installed}")
      endif( ${COLLADADOM_RESULT} )
    endif( ${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/colladadom_installed OR NOT ${colladadom_installed} EQUAL 0 )
  endif( WINDOWS )
else( USESYSTEMLIBS )
use_system_binary( colladadom )

use_prebuilt_binary(colladadom)
use_prebuilt_binary(minizip-ng) # needed for colladadom
use_prebuilt_binary(libxml2)

if (WINDOWS)
    target_link_libraries( ll::minizip-ng INTERFACE ${ARCH_PREBUILT_DIRS_RELEASE}/minizip.lib )
else()
    target_link_libraries( ll::minizip-ng INTERFACE ${ARCH_PREBUILT_DIRS_RELEASE}/libminizip.a )
endif()

if (WINDOWS)
    target_link_libraries( ll::libxml INTERFACE ${ARCH_PREBUILT_DIRS_RELEASE}/libxml2.lib Bcrypt.lib)
else()
    target_link_libraries( ll::libxml INTERFACE ${ARCH_PREBUILT_DIRS_RELEASE}/libxml2.a)
endif()
endif( USESYSTEMLIBS )

target_include_directories( ll::colladadom SYSTEM INTERFACE
        ${LIBS_PREBUILT_DIR}/include/collada
        ${LIBS_PREBUILT_DIR}/include/collada/1.4
        )
if (WINDOWS)
    target_link_libraries(ll::colladadom INTERFACE ${ARCH_PREBUILT_DIRS_RELEASE}/libcollada14dom23-s.lib ll::libxml ll::minizip-ng )
else ()
    target_link_libraries(ll::colladadom INTERFACE collada14dom ll::boost ll::libxml ll::minizip-ng)
endif()
