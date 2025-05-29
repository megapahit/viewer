# -*- cmake -*-
if (XXHASH_CMAKE_INCLUDED)
  return()
endif (XXHASH_CMAKE_INCLUDED)
set (XXHASH_CMAKE_INCLUDED TRUE)

include(Prebuilt)

include(FindPkgConfig)
pkg_check_modules(Xxhash REQUIRED libxxhash)
return ()

use_prebuilt_binary(xxhash)
