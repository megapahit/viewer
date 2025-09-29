# -*- cmake -*-
include_guard()

include(Prebuilt)

include(FindPkgConfig)
pkg_check_modules(Xxhash REQUIRED libxxhash)
return ()

use_prebuilt_binary(xxhash)
