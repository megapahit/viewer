# -*- cmake -*-
include_guard()

include(Prebuilt)
include(Linking)

add_library( ll::openjpeg INTERFACE IMPORTED )

include(FindPkgConfig)
pkg_check_modules(Openjpeg REQUIRED libopenjp2)
target_include_directories(ll::openjpeg SYSTEM INTERFACE ${Openjpeg_INCLUDE_DIRS})
target_link_directories(ll::openjpeg INTERFACE ${Openjpeg_LIBRARY_DIRS})
target_link_libraries(ll::openjpeg INTERFACE ${Openjpeg_LIBRARIES})

target_include_directories( ll::openjpeg SYSTEM INTERFACE ${LIBS_PREBUILT_DIR}/include)
