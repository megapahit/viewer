# -*- cmake -*-
include(Prebuilt)

add_library(ll::glext INTERFACE IMPORTED)
#use_system_binary(glext)
if (WINDOWS)
use_prebuilt_binary(glext)
endif ()


