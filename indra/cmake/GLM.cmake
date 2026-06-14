# -*- cmake -*-
include(Prebuilt)

add_library( ll::glm INTERFACE IMPORTED )

#use_system_binary( glm )
if (${LINUX_DISTRO} MATCHES debian OR CMAKE_OSX_ARCHITECTURES MATCHES x86_64 OR ($ENV{MSYSTEM_CARCH} MATCHES aarch64))
use_prebuilt_binary(glm)
elseif (NOT WINDOWS)
  find_package( glm REQUIRED )
endif ()
