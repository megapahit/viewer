# -*- cmake -*-
include(Prebuilt)

if (NOT USESYSTEMLIBS)

use_prebuilt_binary(tinygltf)

set(TINYGLTF_INCLUDE_DIR ${LIBS_PREBUILT_DIR}/include/tinygltf)

endif (NOT USESYSTEMLIBS)

