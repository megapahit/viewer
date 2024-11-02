# -*- cmake -*-
include(Linking)
include(Prebuilt)

include_guard()

set(USE_OPENAL ON CACHE BOOL "Enable OpenAL")
# ND: To streamline arguments passed, switch from OPENAL to USE_OPENAL
# To not break all old build scripts convert old arguments but warn about it
if(OPENAL)
  message( WARNING "Use of the OPENAL argument is deprecated, please switch to USE_OPENAL")
  set(USE_OPENAL ${OPENAL})
endif()

if (USE_OPENAL)
  add_library( ll::openal INTERFACE IMPORTED )

  if (NOT (USE_AUTOBUILD_3P OR USE_CONAN))
    target_compile_definitions( ll::openal INTERFACE LL_OPENAL=1)
    include(FindPkgConfig)
    pkg_check_modules(Openal REQUIRED freealut)
    target_include_directories(ll::openal SYSTEM INTERFACE ${Openal_INCLUDE_DIRS})
    target_link_directories(ll::openal INTERFACE ${Openal_LIBRARY_DIRS})
    target_link_libraries(ll::openal INTERFACE ${Openal_LIBRARIES})
    return ()
  endif ()

  target_include_directories( ll::openal SYSTEM INTERFACE "${LIBS_PREBUILT_DIR}/include/AL")
  target_compile_definitions( ll::openal INTERFACE LL_OPENAL=1)
  use_prebuilt_binary(openal)

  if(WINDOWS)
    target_link_libraries( ll::openal INTERFACE
            OpenAL32
            alut
            )
  elseif(LINUX)
    target_link_libraries( ll::openal INTERFACE
            openal
            alut
            )
  else()
    target_link_libraries( ll::openal INTERFACE
            openal
            alut
            )
  endif()
endif ()
