# -*- cmake -*-

add_subdirectory(base)
if (NOT (CMAKE_SYSTEM_NAME MATCHES FreeBSD))
add_subdirectory(cef)
endif ()
add_subdirectory(libvlc)
add_subdirectory(example)

if (FALSE)
    add_subdirectory(gstreamer10)
endif ()
