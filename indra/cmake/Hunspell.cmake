# -*- cmake -*-
include(Prebuilt)

include_guard()
if (USE_AUTOBUILD_3P OR USE_CONAN)
use_prebuilt_binary(dictionaries)
endif ()

add_library( ll::hunspell INTERFACE IMPORTED )
use_system_binary(hunspell)
use_prebuilt_binary(libhunspell)
if (WINDOWS)
  target_link_libraries( ll::hunspell INTERFACE libhunspell)
elseif(DARWIN)
  target_link_libraries( ll::hunspell INTERFACE hunspell-1.3)
elseif(LINUX)
  target_link_libraries( ll::hunspell INTERFACE hunspell-1.3)
endif()
target_include_directories( ll::hunspell SYSTEM INTERFACE ${LIBS_PREBUILT_DIR}/include/hunspell)
