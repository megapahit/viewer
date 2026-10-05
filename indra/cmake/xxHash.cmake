# -*- cmake -*-
include_guard()

include(Prebuilt)

include(FindPkgConfig)
pkg_check_modules(Xxhash REQUIRED libxxhash)
if (DARWIN_USE_HOMEBREW AND Xxhash_INCLUDE_DIRS)
  # hbxxh.cpp includes <xxhash.h>. Homebrew does not put that directory on the default search path.
  include_directories(SYSTEM ${Xxhash_INCLUDE_DIRS})
endif ()
return ()

use_prebuilt_binary(xxhash)
