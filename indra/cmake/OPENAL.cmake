# -*- cmake -*-
include(Linking)
include(Prebuilt)

include_guard()

# ND: Turn this off by default, the openal code in the viewer isn't very well maintained, seems
# to have memory leaks, has no option to play music streams
# It probably makes sense to to completely remove it

set(USE_OPENAL ON CACHE BOOL "Enable OpenAL")
# ND: To streamline arguments passed, switch from OPENAL to USE_OPENAL
# To not break all old build scripts convert old arguments but warn about it
if(OPENAL)
  message( WARNING "Use of the OPENAL argument is deprecated, please switch to USE_OPENAL")
  set(USE_OPENAL ${OPENAL})
endif()

if (USE_OPENAL)
  add_library( ll::openal INTERFACE IMPORTED )
  if (${LINUX_DISTRO} MATCHES freedesktop)
  target_include_directories( ll::openal SYSTEM INTERFACE "${LIBS_PREBUILT_DIR}/include/AL")
  endif ()
  target_compile_definitions( ll::openal INTERFACE LL_OPENAL=1)
  if (${LINUX_DISTRO} MATCHES freedesktop)
  use_prebuilt_binary(openal)
      file(REMOVE
          ${ARCH_PREBUILT_DIRS_RELEASE}/libopenal.so
          ${ARCH_PREBUILT_DIRS_RELEASE}/libopenal.so.1
          ${ARCH_PREBUILT_DIRS_RELEASE}/libopenal.so.1.24.2
          ${LIBS_PREBUILT_DIR}/include/AL/al.h
          ${LIBS_PREBUILT_DIR}/include/AL/alc.h
          ${LIBS_PREBUILT_DIR}/include/AL/alext.h
          ${LIBS_PREBUILT_DIR}/include/AL/efx-creative.h
          ${LIBS_PREBUILT_DIR}/include/AL/efx-presets.h
          ${LIBS_PREBUILT_DIR}/include/AL/efx.h
  )
  endif ()

  if (FALSE)
  find_library(OPENAL_LIBRARY
      NAMES
      OpenAL32
      openal
      OpenAL32.lib
      libopenal.dylib
      libopenal.so
      PATHS "${ARCH_PREBUILT_DIRS_RELEASE}" REQUIRED NO_DEFAULT_PATH)
  endif ()

  include(FindPkgConfig)
  if (${LINUX_DISTRO} MATCHES freedesktop)
      pkg_search_module(Openal REQUIRED openal)
  find_library(ALUT_LIBRARY
      NAMES
      alut
      alut.lib
      libalut.dylib
      libalut.so
      PATHS "${ARCH_PREBUILT_DIRS_RELEASE}" REQUIRED NO_DEFAULT_PATH)

  target_link_libraries(ll::openal INTERFACE ${OPENAL_LIBRARY} ${ALUT_LIBRARY})
  else ()
      pkg_search_module(Openal REQUIRED freealut)
      if (DARWIN)
        include(DarwinPackages)
      endif ()
      # Homebrew freealut is built against Apple's OpenAL.framework. The
      # viewer links openal-soft, so alutInit and alGenSources talk to
      # different libraries and wind playback never gets a source.
      if (DARWIN_USE_HOMEBREW)
        set(_openal_soft "${DARWIN_HOMEBREW_PREFIX}/opt/openal-soft/lib/libopenal.1.dylib")
        find_library(_homebrew_alut
          NAMES libalut.0.dylib alut
          PATHS ${Openal_LIBRARY_DIRS}
          NO_DEFAULT_PATH
          REQUIRED)
        execute_process(
          COMMAND otool -L "${_homebrew_alut}"
          OUTPUT_VARIABLE _alut_deps
          COMMAND_ERROR_IS_FATAL ANY)
        if (_alut_deps MATCHES "/System/Library/Frameworks/OpenAL.framework/Versions/A/OpenAL")
          set(_alut_dir "${CMAKE_BINARY_DIR}/openal-compat")
          file(MAKE_DIRECTORY "${_alut_dir}")
          set(_alut_rewritten "${_alut_dir}/libalut.0.dylib")
          execute_process(COMMAND "${CMAKE_COMMAND}" -E copy "${_homebrew_alut}" "${_alut_rewritten}")
          execute_process(
            COMMAND install_name_tool
              -change /System/Library/Frameworks/OpenAL.framework/Versions/A/OpenAL "${_openal_soft}"
              -id "${_alut_rewritten}"
              "${_alut_rewritten}"
            COMMAND_ERROR_IS_FATAL ANY)
          execute_process(
            COMMAND codesign -f -s - "${_alut_rewritten}"
            COMMAND_ERROR_IS_FATAL ANY)
          list(TRANSFORM Openal_LIBRARIES REPLACE "^alut$" "${_alut_rewritten}")
          message(STATUS "freealut retargeted onto openal-soft: ${_alut_rewritten}")
        endif ()
      endif ()
  endif ()

  target_include_directories(ll::openal SYSTEM INTERFACE ${Openal_INCLUDE_DIRS})
  target_link_directories(ll::openal INTERFACE ${Openal_LIBRARY_DIRS})
  target_link_libraries(ll::openal INTERFACE ${Openal_LIBRARIES})

endif ()
