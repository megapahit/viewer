# -*- cmake -*-
cmake_minimum_required( VERSION 3.13 FATAL_ERROR )

include(Linking)
include( Prebuilt )
include_guard()

add_library( ll::SDL2 INTERFACE IMPORTED )

if (USESYSTEMLIBS AND NOT (WINDOWS OR DARWIN))
  include(FindPkgConfig)
  pkg_check_modules(Sdl2 REQUIRED sdl2)
  target_include_directories(ll::SDL2 SYSTEM INTERFACE ${Sdl2_INCLUDE_DIRS})
  target_link_directories(ll::SDL2 INTERFACE ${Sdl2_LIBRARY_DIRS})
  target_link_libraries(ll::SDL2 INTERFACE ${Sdl2_LIBRARIES})
  return ()
endif (USESYSTEMLIBS AND NOT (WINDOWS OR DARWIN))

use_system_binary( SDL2 )
use_prebuilt_binary( SDL2 )

find_library( SDL2_LIBRARY
    NAMES SDL2
    PATHS "${LIBS_PREBUILT_DIR}/lib/release" REQUIRED)

target_link_libraries( ll::SDL2 INTERFACE "${SDL2_LIBRARY}" )
target_include_directories( ll::SDL2 SYSTEM INTERFACE "${LIBS_PREBUILT_DIR}/include/SDL2" )

