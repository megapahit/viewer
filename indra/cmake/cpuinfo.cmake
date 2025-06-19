include(Prebuilt)

add_library( ll::cpuinfo INTERFACE IMPORTED )

use_system_binary(cpuinfo)
