# -*- cmake -*-
include(Prebuilt)

add_library( ll::glm INTERFACE IMPORTED )

#use_system_binary( glm )
if (${LINUX_DISTRO} MATCHES debian)
use_prebuilt_binary(glm)
elseif (NOT WINDOWS)
  find_package( glm REQUIRED )
endif ()
