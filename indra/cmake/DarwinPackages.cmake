# -*- cmake -*-
#
# Choose the Darwin dependency provider.
# MacPorts stays the default when Boost 1.88 is installed at the path the
# rest of the build has always used. Homebrew is selected otherwise, or when
# DARWIN_PACKAGE_MANAGER is set to "homebrew".
include_guard(GLOBAL)

if (NOT DARWIN)
  return()
endif ()

if (EXISTS "/opt/local/libexec/boost/1.88")
  set(_darwin_pkg_default macports)
else ()
  set(_darwin_pkg_default homebrew)
endif ()

set(DARWIN_PACKAGE_MANAGER "${_darwin_pkg_default}" CACHE STRING
  "Darwin dependency provider: macports or homebrew")
set_property(CACHE DARWIN_PACKAGE_MANAGER PROPERTY STRINGS macports homebrew)

if (DARWIN_PACKAGE_MANAGER STREQUAL "homebrew")
  set(DARWIN_USE_HOMEBREW TRUE)
  find_program(BREW_EXECUTABLE brew REQUIRED)
  execute_process(
    COMMAND "${BREW_EXECUTABLE}" --prefix
    OUTPUT_VARIABLE DARWIN_HOMEBREW_PREFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
    )
  execute_process(
    COMMAND "${BREW_EXECUTABLE}" --prefix boost
    OUTPUT_VARIABLE DARWIN_BOOST_PREFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
    )
  execute_process(
    COMMAND "${BREW_EXECUTABLE}" --prefix libnghttp2
    OUTPUT_VARIABLE DARWIN_NGHTTP2_PREFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
    )
  execute_process(
    COMMAND "${BREW_EXECUTABLE}" --prefix apr
    OUTPUT_VARIABLE DARWIN_APR_PREFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
    )
  execute_process(
    COMMAND "${BREW_EXECUTABLE}" --prefix apr-util
    OUTPUT_VARIABLE DARWIN_APRUTIL_PREFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
    )
  execute_process(
    COMMAND "${BREW_EXECUTABLE}" --prefix icu4c@78
    OUTPUT_VARIABLE DARWIN_ICU_PREFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
    )
  # The macOS apr-1.pc points at /usr/include/apr-1, which is not on disk
  # (headers live in the SDK). Prefer the Homebrew apr pkg-config files.
  # openal-soft is keg-only, so its openal.pc is absent from the default
  # pkg-config path until this directory is prepended. freealut.pc requires it.
  set(_darwin_pkgconfig_dirs
    "${DARWIN_APR_PREFIX}/lib/pkgconfig"
    "${DARWIN_APRUTIL_PREFIX}/lib/pkgconfig"
    )
  execute_process(
    COMMAND "${BREW_EXECUTABLE}" --prefix openal-soft
    OUTPUT_VARIABLE _darwin_openal_prefix
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _darwin_openal_rc
    )
  if (_darwin_openal_rc EQUAL 0 AND EXISTS "${_darwin_openal_prefix}/lib/pkgconfig")
    list(APPEND _darwin_pkgconfig_dirs "${_darwin_openal_prefix}/lib/pkgconfig")
  endif ()
  list(JOIN _darwin_pkgconfig_dirs ":" _darwin_pkgconfig_prefix)
  set(ENV{PKG_CONFIG_PATH} "${_darwin_pkgconfig_prefix}:$ENV{PKG_CONFIG_PATH}")
  if (NOT CMAKE_PREFIX_PATH MATCHES "(^|;)${DARWIN_HOMEBREW_PREFIX}(;|$)")
    list(PREPEND CMAKE_PREFIX_PATH "${DARWIN_HOMEBREW_PREFIX}")
  endif ()
  # llmath headers include glm directly. A private include dir keeps Homebrew
  # OpenSSL and other linked headers off the global search path.
  if (EXISTS "${DARWIN_HOMEBREW_PREFIX}/include/glm/vec3.hpp")
    set(_darwin_glm_include "${CMAKE_BINARY_DIR}/homebrew-includes")
    file(MAKE_DIRECTORY "${_darwin_glm_include}")
    file(REMOVE "${_darwin_glm_include}/glm")
    file(CREATE_LINK "${DARWIN_HOMEBREW_PREFIX}/include/glm" "${_darwin_glm_include}/glm" SYMBOLIC)
    include_directories(SYSTEM "${_darwin_glm_include}")
  endif ()
  set(DARWIN_BOOST_LIBRARY_SUFFIX "")
else ()
  set(DARWIN_USE_HOMEBREW FALSE)
  set(DARWIN_BOOST_PREFIX "/opt/local/libexec/boost/1.88")
  set(DARWIN_BOOST_LIBRARY_SUFFIX "-mt")
endif ()

message(STATUS "Darwin package manager: ${DARWIN_PACKAGE_MANAGER} (Boost ${DARWIN_BOOST_PREFIX}, suffix '${DARWIN_BOOST_LIBRARY_SUFFIX}')")
