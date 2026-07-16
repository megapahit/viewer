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
  if (USE_FLATPAK)
  target_include_directories( ll::openal SYSTEM INTERFACE "${LIBS_PREBUILT_DIR}/include/AL")
  endif ()
  target_compile_definitions( ll::openal INTERFACE LL_OPENAL=1)
  if (USE_FLATPAK)
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
      libopenal.dylib
      libopenal.so
      PATHS "${ARCH_PREBUILT_DIRS_RELEASE}" REQUIRED NO_DEFAULT_PATH)
  endif ()

  include(FindPkgConfig)
  if (USE_FLATPAK)
      pkg_search_module(Openal REQUIRED openal)
  find_library(ALUT_LIBRARY
      NAMES
      alut
      libalut.dylib
      libalut.so
      PATHS "${ARCH_PREBUILT_DIRS_RELEASE}" REQUIRED NO_DEFAULT_PATH)

  target_link_libraries(ll::openal INTERFACE ${OPENAL_LIBRARY} ${ALUT_LIBRARY})
  else ()
      pkg_search_module(Openal REQUIRED freealut)
  endif ()

  target_include_directories(ll::openal SYSTEM INTERFACE ${Openal_INCLUDE_DIRS})
  target_link_directories(ll::openal INTERFACE ${Openal_LIBRARY_DIRS})
  target_link_libraries(ll::openal INTERFACE ${Openal_LIBRARIES})

endif ()
