# -*- cmake -*-
include(Linking)
include(Prebuilt)

include_guard()
add_library( ll::vorbis INTERFACE IMPORTED )

if (NOT (USE_AUTOBUILD_3P OR USE_CONAN))
  include(FindPkgConfig)
  pkg_check_modules(Vorbis REQUIRED ogg vorbis vorbisenc vorbisfile)
  target_include_directories(ll::vorbis SYSTEM INTERFACE ${Vorbis_INCLUDE_DIRS})
  target_link_directories(ll::vorbis INTERFACE ${Vorbis_LIBRARY_DIRS})
  target_link_libraries(ll::vorbis INTERFACE ${Vorbis_LIBRARIES})
  return ()
endif ()

use_system_binary(vorbis)
use_prebuilt_binary(ogg_vorbis)
target_include_directories( ll::vorbis SYSTEM INTERFACE ${LIBS_PREBUILT_DIR}/include )

if (WINDOWS)
  target_link_libraries(ll::vorbis INTERFACE
        optimized ${ARCH_PREBUILT_DIRS_RELEASE}/libogg.lib
        debug ${ARCH_PREBUILT_DIRS_DEBUG}/libogg.lib
        optimized ${ARCH_PREBUILT_DIRS_RELEASE}/libvorbisenc.lib
        debug ${ARCH_PREBUILT_DIRS_DEBUG}/libvorbisenc.lib
        optimized ${ARCH_PREBUILT_DIRS_RELEASE}/libvorbisfile.lib
        debug ${ARCH_PREBUILT_DIRS_DEBUG}/libvorbisfile.lib
        optimized ${ARCH_PREBUILT_DIRS_RELEASE}/libvorbis.lib
        debug ${ARCH_PREBUILT_DIRS_DEBUG}/libvorbis.lib
    )
else (WINDOWS)
  target_link_libraries(ll::vorbis INTERFACE vorbisfile vorbis ogg vorbisenc )
endif (WINDOWS)

