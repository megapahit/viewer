# -*- cmake -*-
include(Prebuilt)

if (NOT (DARWIN OR WINDOWS))
  #use_prebuilt_binary(libuuid)
  add_library( ll::fontconfig INTERFACE IMPORTED )

  find_package(Fontconfig REQUIRED)
  target_link_libraries( ll::fontconfig INTERFACE  Fontconfig::Fontconfig )
endif ()

if( FALSE )
  use_prebuilt_binary(libhunspell)
endif()

if (DARWIN OR (${LINUX_DISTRO} MATCHES freedesktop))
use_prebuilt_binary(nanosvg)
endif ()
use_prebuilt_binary(viewer-fonts)
use_prebuilt_binary(google-fonts)
use_prebuilt_binary(emoji_shortcodes)

include(LSLDefinitions)
