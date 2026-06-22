# -*- cmake -*-

include(Variables)
include(Prebuilt)
include(FindOpenGL)

if (USE_FLATPAK)
  add_library(ll::glu INTERFACE IMPORTED)
  use_prebuilt_binary(glu)
  target_link_libraries(ll::glu INTERFACE GLU)
endif ()
