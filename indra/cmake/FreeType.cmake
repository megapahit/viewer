# -*- cmake -*-
include(Prebuilt)
include(Linking)

include_guard()
add_library( ll::freetype INTERFACE IMPORTED )

include(FindPkgConfig)
pkg_check_modules(Freetype REQUIRED freetype2)
target_include_directories( ll::freetype SYSTEM INTERFACE ${Freetype_INCLUDE_DIRS} )
target_link_directories( ll::freetype INTERFACE ${Freetype_LIBRARY_DIRS} )
target_link_libraries( ll::freetype INTERFACE ${Freetype_LIBRARIES} )
return ()

use_system_binary(freetype)
use_prebuilt_binary(freetype)
target_include_directories( ll::freetype SYSTEM INTERFACE  ${LIBS_PREBUILT_DIR}/include/freetype2/)

if (WINDOWS)
    target_link_libraries( ll::freetype INTERFACE ${ARCH_PREBUILT_DIRS_RELEASE}/freetype.lib)
else()
    target_link_libraries( ll::freetype INTERFACE ${ARCH_PREBUILT_DIRS_RELEASE}/libfreetype.a)
endif()

