# -*- cmake -*-
include(Prebuilt)

add_library( ll::glm INTERFACE IMPORTED )

if (NOT USESYSTEMLIBS)
use_system_binary( glm )
elseif (NOT (${LINUX_DISTRO} MATCHES debian OR (${LINUX_DISTRO} MATCHES ubuntu) OR (${LINUX_DISTRO} MATCHES opensuse-tumbleweed)))
  find_package( glm REQUIRED )
endif (NOT USESYSTEMLIBS)

if (${LINUX_DISTRO} MATCHES debian OR (${LINUX_DISTRO} MATCHES ubuntu) OR (${LINUX_DISTRO} MATCHES opensuse-tumbleweed) OR NOT USESYSTEMLIBS)
use_prebuilt_binary(glm)
endif ()
