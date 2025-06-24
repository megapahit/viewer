# -*- cmake -*-
include(Prebuilt)

set(NVAPI ON CACHE BOOL "Use NVAPI.")

if (NVAPI)
  if (WINDOWS AND NOT ($ENV{MSYSTEM_CARCH} MATCHES aarch64))
    add_library( ll::nvapi INTERFACE IMPORTED )
    target_link_libraries( ll::nvapi INTERFACE nvapi)
    use_prebuilt_binary(nvapi)
  endif ()
endif (NVAPI)

