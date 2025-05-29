# -*- cmake -*-
include(Prebuilt)

add_library( ll::glh_linear INTERFACE IMPORTED )
target_include_directories( ll::glh_linear SYSTEM INTERFACE ${LIBS_PREBUILT_DIR}/include)

#use_system_binary( glh_linear )
use_prebuilt_binary(glh_linear)
