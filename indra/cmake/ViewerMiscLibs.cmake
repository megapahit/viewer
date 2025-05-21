# -*- cmake -*-
include(Prebuilt)

if (NOT (DARWIN OR WINDOWS))
  add_library( ll::fontconfig INTERFACE IMPORTED )

  find_package(Fontconfig REQUIRED)
  target_link_libraries( ll::fontconfig INTERFACE  Fontconfig::Fontconfig )
endif ()

if (FALSE)
if( NOT USE_CONAN )
  use_prebuilt_binary(libhunspell)
endif()

use_prebuilt_binary(slvoice)
endif (FALSE)

if (${LINUX_DISTRO} MATCHES debian OR DARWIN OR WINDOWS)
use_prebuilt_binary(nanosvg)
endif ()
use_prebuilt_binary(viewer-fonts)
use_prebuilt_binary(emoji_shortcodes)
