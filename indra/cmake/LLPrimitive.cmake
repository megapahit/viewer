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

if (LINUX AND NOT (${LINUX_DISTRO} MATCHES debian AND CMAKE_SYSTEM_PROCESSOR MATCHES aarch64) OR CMAKE_SYSTEM_NAME MATCHES FreeBSD)
  # Build of the collada-dom for Linux and FreeBSD is done in
  # indra/llprimitive/CMakeLists.txt
  return()
else ()
  include(FindPkgConfig)
  pkg_check_modules(Minizip REQUIRED minizip)
  pkg_check_modules(Libxml2 REQUIRED libxml-2.0)
  target_link_libraries( ll::minizip-ng INTERFACE ${Minizip_LIBRARIES} )
  target_link_libraries( ll::libxml INTERFACE ${Libxml2_LIBRARIES} )
  if (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/colladadom_installed OR NOT ${colladadom_installed} EQUAL 0)
    if (NOT EXISTS ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8.tar.gz)
      file(DOWNLOAD
        https://github.com/secondlife/3p-colladadom/archive/refs/tags/v2.3-r8.tar.gz
        ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8.tar.gz
        )
    endif ()
    file(ARCHIVE_EXTRACT
      INPUT ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8.tar.gz
      DESTINATION ${CMAKE_BINARY_DIR}
      )
    if (WINDOWS)
      execute_process(
        COMMAND sed -i "s/SHARED/STATIC/" 1.4/CMakeLists.txt
        COMMAND sed -i "/#include <cstdarg>/a #define WIN32" dae/daeUtils.cpp
        COMMAND sed -i "/using namespace cdom;/a namespace boost{void boost::throw_exception(class std::exception const &){}}" dae/daeURI.cpp
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8/src
        )
    else ()
      execute_process(
        COMMAND sed -i "" -e "s/SHARED/STATIC/" src/1.4/CMakeLists.txt
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8
        )
    endif ()
    if (DARWIN)
      set(BOOST_CFLAGS -I${Libxml2_LIBRARY_DIRS}exec/boost/1.87/include)
      set(BOOST_LIBS -L${Minizip_LIBRARY_DIRS}exec/boost/1.87/lib)
      set(BOOST_LIBRARY_SUFFIX -mt)
    elseif (WINDOWS)
      set(BOOST_CFLAGS -I${prefix_result}/../include)
      set(BOOST_LIBS -L${prefix_result})
      if ($ENV{MSYSTEM_CARCH} MATCHES aarch64)
        set(BOOST_LIBRARY_SUFFIX -vc143-mt-a64-1_88)
      else ()
        set(BOOST_LIBRARY_SUFFIX -vc143-mt-x64-1_88)
      endif ()
    endif ()
    file(MAKE_DIRECTORY ${LIBS_PREBUILT_DIR}/include/collada/1.4)
    try_compile(COLLADADOM_RESULT
      PROJECT colladadom
      SOURCE_DIR ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8
      BINARY_DIR ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8
      TARGET collada14dom
      CMAKE_FLAGS
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DCMAKE_OSX_ARCHITECTURES:STRING=${CMAKE_OSX_ARCHITECTURES}
        -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=${CMAKE_OSX_DEPLOYMENT_TARGET}
        -DCMAKE_INSTALL_PREFIX:PATH=${LIBS_PREBUILT_DIR}
        -DCMAKE_CXX_STANDARD:STRING=17
        -DCMAKE_CXX_FLAGS:STRING=-I${Minizip_INCLUDE_DIRS}
        -DBoost_CFLAGS:STRING=${BOOST_CFLAGS}
        -DEXTRA_COMPILE_FLAGS:STRING=-I${Libxml2_INCLUDE_DIRS}
        "-DCMAKE_SHARED_LINKER_FLAGS:STRING=-L${Minizip_LIBRARY_DIRS} ${BOOST_LIBS}"
        -DBoost_FILESYSTEM_LIBRARY:STRING=boost_filesystem${BOOST_LIBRARY_SUFFIX}
        -DBoost_SYSTEM_LIBRARY:STRING=boost_system${BOOST_LIBRARY_SUFFIX}
        -DZLIB_LIBRARIES:STRING=${Libxml2_LIBRARIES}
        -DOPT_COLLADA14:BOOL=ON
        -DCOLLADA_DOM_INCLUDE_INSTALL_DIR:PATH=${LIBS_PREBUILT_DIR}/include/collada
      )
    if (WINDOWS)
      execute_process(
        COMMAND MSBuild.exe ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8/Project.sln -p:Configuration=${CMAKE_BUILD_TYPE}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8
        OUTPUT_VARIABLE colladadom_installed
        )
      file(REMOVE_RECURSE ${LIBS_PREBUILT_DIR}/include/collada)
      file(
        COPY ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8/include
        DESTINATION ${LIBS_PREBUILT_DIR}/include
        )
      file(RENAME
        ${LIBS_PREBUILT_DIR}/include/include
        ${LIBS_PREBUILT_DIR}/include/collada
        )
      file(MAKE_DIRECTORY ${ARCH_PREBUILT_DIRS_RELEASE})
      file(RENAME
        ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8/src/1.4/${CMAKE_BUILD_TYPE}/collada14dom.lib
        ${ARCH_PREBUILT_DIRS_RELEASE}/libcollada14dom23-s.lib
        )
    elseif (${COLLADADOM_RESULT})
      execute_process(
        COMMAND ${CMAKE_MAKE_PROGRAM} install
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/3p-colladadom-2.3-r8
        OUTPUT_VARIABLE colladadom_installed
        )
      file(RENAME
        ${ARCH_PREBUILT_DIRS}/libcollada14dom.a
        ${ARCH_PREBUILT_DIRS_RELEASE}/libcollada14dom.a
        )
    endif ()
    file(WRITE ${PREBUILD_TRACKING_DIR}/colladadom_installed "${colladadom_installed}")
  endif ()
endif ()

if (FALSE)
use_system_binary( colladadom )

use_prebuilt_binary(colladadom)
use_prebuilt_binary(minizip-ng) # needed for colladadom
use_prebuilt_binary(libxml2)

find_library(MINIZIPNG_LIBRARY
    NAMES
    minizip.lib
    libminizip.a
    PATHS "${ARCH_PREBUILT_DIRS_RELEASE}" REQUIRED NO_DEFAULT_PATH)

target_link_libraries(ll::minizip-ng INTERFACE ${MINIZIPNG_LIBRARY})

find_library(LIBXML2_LIBRARY
    NAMES
    libxml2.lib
    libxml2.a
    PATHS "${ARCH_PREBUILT_DIRS_RELEASE}" REQUIRED NO_DEFAULT_PATH)

target_link_libraries(ll::libxml INTERFACE ${LIBXML2_LIBRARY})

if (WINDOWS)
    target_link_libraries( ll::libxml INTERFACE Bcrypt.lib)
endif()
endif (FALSE)

target_include_directories( ll::colladadom SYSTEM INTERFACE
        ${LIBS_PREBUILT_DIR}/include/collada
        ${LIBS_PREBUILT_DIR}/include/collada/1.4
        )

find_library(COLLADADOM_LIBRARY
    NAMES
    libcollada14dom23-s.lib
    collada14dom
    PATHS "${ARCH_PREBUILT_DIRS_RELEASE}" REQUIRED NO_DEFAULT_PATH)

target_link_libraries(ll::colladadom INTERFACE ${COLLADADOM_LIBRARY} ll::boost ll::libxml ll::minizip-ng)
