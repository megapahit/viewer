# -*- cmake -*-
include(Prebuilt)

add_library( ll::glm INTERFACE IMPORTED )

#use_system_binary( glm )
if (${LINUX_DISTRO} MATCHES debian OR (${LINUX_DISTRO} MATCHES ubuntu) OR (${LINUX_DISTRO} MATCHES opensuse-tumbleweed))
use_prebuilt_binary(glm)
else ()
  find_package( glm REQUIRED )
endif ()
