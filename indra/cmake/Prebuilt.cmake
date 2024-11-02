# -*- cmake -*-
include_guard()

include(FindAutobuild)
if(INSTALL_PROPRIETARY)
  include(FindSCP)
endif(INSTALL_PROPRIETARY)

set(PREBUILD_TRACKING_DIR ${AUTOBUILD_INSTALL_DIR}/cmake_tracking)
# For the library installation process;
# see cmake/Prebuild.cmake for the counterpart code.
if ("${CMAKE_SOURCE_DIR}/../autobuild.xml" IS_NEWER_THAN "${PREBUILD_TRACKING_DIR}/sentinel_installed")
  file(MAKE_DIRECTORY ${PREBUILD_TRACKING_DIR})
  file(WRITE ${PREBUILD_TRACKING_DIR}/sentinel_installed "0")
endif ("${CMAKE_SOURCE_DIR}/../autobuild.xml" IS_NEWER_THAN "${PREBUILD_TRACKING_DIR}/sentinel_installed")

# The use_prebuilt_binary macro handles automated installation of package
# dependencies using autobuild.  The goal is that 'autobuild install' should
# only be run when we know we need to install a new package.  This should be
# the case in a clean checkout, or if autobuild.xml has been updated since the
# last run (encapsulated by the file ${PREBUILD_TRACKING_DIR}/sentinel_installed),
# or if a previous attempt to install the package has failed (the exit status
# of previous attempts is serialized in the file
# ${PREBUILD_TRACKING_DIR}/${_binary}_installed)
macro (use_prebuilt_binary _binary)
    if( NOT DEFINED ${_binary}_installed )
        set( ${_binary}_installed "")
    endif()

    if("${${_binary}_installed}" STREQUAL "" AND EXISTS "${PREBUILD_TRACKING_DIR}/${_binary}_installed")
        file(READ ${PREBUILD_TRACKING_DIR}/${_binary}_installed "${_binary}_installed")
        if(DEBUG_PREBUILT)
            message(STATUS "${_binary}_installed: \"${${_binary}_installed}\"")
        endif(DEBUG_PREBUILT)
    endif("${${_binary}_installed}" STREQUAL "" AND EXISTS "${PREBUILD_TRACKING_DIR}/${_binary}_installed")

    if(${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/${_binary}_installed OR NOT ${${_binary}_installed} EQUAL 0)
        if(DEBUG_PREBUILT)
            message(STATUS "cd ${CMAKE_SOURCE_DIR} && ${AUTOBUILD_EXECUTABLE} install
        --install-dir=${AUTOBUILD_INSTALL_DIR}
        ${_binary} ")
        endif(DEBUG_PREBUILT)
        message(STATUS "Installing ${_binary}...")
        if(USESYSTEMLIBS)
            execute_process(COMMAND xmllint
                --xpath
                "//map/map/map/map/map/map/string[contains(text(),'${_binary}')][contains(text(),'common')]/text()" autobuild.xml
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/..
                OUTPUT_VARIABLE package_url
                OUTPUT_STRIP_TRAILING_WHITESPACE
                )
            if ("${package_url}" STREQUAL "")
                string(TOLOWER ${CMAKE_SYSTEM_NAME} system_name)
                if (${system_name} MATCHES freebsd)
                    set(system_name "linux")
                endif (${system_name} MATCHES freebsd)
                execute_process(COMMAND xmllint
                    --xpath
                    "//map/map/map/map/map/map/string[contains(text(),'${_binary}')][contains(text(),'${system_name}')]/text()" autobuild.xml
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/..
                    OUTPUT_VARIABLE package_url
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    )
            endif ("${package_url}" STREQUAL "")
            string(REGEX REPLACE "^https?://(megapahit.net/downloads|github.com/secondlife|automated-builds-secondlife-com.s3.amazonaws.com/ct2).*/" "" package_name ${package_url})
            file(DOWNLOAD
                ${package_url}
                ${CMAKE_BINARY_DIR}/${package_name}
                )
            file(ARCHIVE_EXTRACT
                INPUT ${CMAKE_BINARY_DIR}/${package_name}
                DESTINATION ${AUTOBUILD_INSTALL_DIR}
                )
            set(${_binary}_installed 0)
        else(USESYSTEMLIBS)
        execute_process(COMMAND "${AUTOBUILD_EXECUTABLE}"
                install
                -A${ADDRESS_SIZE}
                --skip-source-environment
                --install-dir=${AUTOBUILD_INSTALL_DIR}
                ${_binary}
                WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                RESULT_VARIABLE ${_binary}_installed
                )
        endif(USESYSTEMLIBS)
        file(WRITE ${PREBUILD_TRACKING_DIR}/${_binary}_installed "${${_binary}_installed}")
    endif(${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/${_binary}_installed OR NOT ${${_binary}_installed} EQUAL 0)

    if(NOT ${_binary}_installed EQUAL 0)
        message(FATAL_ERROR
                "Failed to download or unpack prebuilt '${_binary}'."
                " Process returned ${${_binary}_installed}.")
    endif (NOT ${_binary}_installed EQUAL 0)
endmacro (use_prebuilt_binary _binary)

#Sadly we need a macro here, otherwise the return() will not properly work
macro ( use_system_binary package )
  if( USE_CONAN )
    target_link_libraries( ll::${package} INTERFACE CONAN_PKG::${package} )
    foreach( extra_pkg "${ARGN}" )
      if( extra_pkg )
        target_link_libraries( ll::${package} INTERFACE CONAN_PKG::${extra_pkg} )
      endif()
    endforeach()
    return()
  elseif( NOT USE_AUTOBUILD_3P )
    include(FindPkgConfig)
    pkg_check_modules(${package} ${package})
    if( ${package}_FOUND )
      target_link_directories( ll::${package} INTERFACE ${${package}_LIBRARY_DIRS} )
    else()
      pkg_check_modules(${package} lib${package})
      if( ${package}_FOUND )
        target_link_directories( ll::${package} INTERFACE ${${package}_LIBRARY_DIRS} )
      else()
        find_package( ${package} REQUIRED )
      endif()
    endif()
    target_include_directories( ll::${package} SYSTEM INTERFACE ${${package}_INCLUDE_DIRS} )
    target_link_libraries( ll::${package} INTERFACE ${${package}_LIBRARIES} )
    return()
  endif()
endmacro()

find_package(Patch)

# Download the third party software archive and patch it.
macro(prepare_thirdparty url filename dirname patch hash)
  file(DOWNLOAD ${url} ${filename} EXPECTED_HASH SHA512=${hash})
  file(ARCHIVE_EXTRACT INPUT ${filename}
	  DESTINATION ${CMAKE_SOURCE_DIR}/../..)
  execute_process(COMMAND ${Patch_EXECUTABLE} "-p1" "-i"
	  ${CMAKE_SOURCE_DIR}/../patches/${patch}
	  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/../../${dirname})
endmacro()
