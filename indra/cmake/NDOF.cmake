# -*- cmake -*-
include(Prebuilt)

set(NDOF ON CACHE BOOL "Use NDOF space navigator joystick library.")

include_guard()
add_library( ll::ndof INTERFACE IMPORTED )

if (NDOF)
  if (NOT USESYSTEMLIBS)
  if (WINDOWS OR DARWIN)
    use_prebuilt_binary(libndofdev)
  elseif (LINUX)
    use_prebuilt_binary(open-libndofdev)
  endif (WINDOWS OR DARWIN)
  endif (NOT USESYSTEMLIBS)

  if (WINDOWS)
    target_link_libraries( ll::ndof INTERFACE libndofdev)
  elseif (DARWIN OR LINUX)
    target_link_libraries( ll::ndof INTERFACE ndofdev)
  endif (WINDOWS)
  target_compile_definitions( ll::ndof INTERFACE LIB_NDOF=1)
else()
  add_compile_options(-ULIB_NDOF)
endif (NDOF)
