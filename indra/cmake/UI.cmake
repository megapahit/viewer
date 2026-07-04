# -*- cmake -*-
include(Prebuilt)
include(FreeType)
include(GLIB)

add_library( ll::uilibraries INTERFACE IMPORTED )

if (LINUX OR CMAKE_SYSTEM_NAME MATCHES FreeBSD)
  if (USE_FLATPAK)
  use_prebuilt_binary(fltk)
  endif ()
  target_compile_definitions(ll::uilibraries INTERFACE LL_FLTK=1 LL_X11=1 )

  if( USE_CONAN )
    return()
  endif()

  if (USE_FLATPAK OR (${LINUX_DISTRO} MATCHES debian) OR (${LINUX_DISTRO} MATCHES ubuntu))
    include(FindPkgConfig)
    pkg_check_modules(CAIRO REQUIRED cairo)
    target_include_directories(ll::uilibraries SYSTEM INTERFACE ${CAIRO_INCLUDE_DIRS})
  endif ()

  target_link_libraries( ll::uilibraries INTERFACE
          fltk
          Xrender
          Xcursor
          Xfixes
          Xext
          Xft
          Xinerama
          ll::fontconfig
          ll::freetype
          ll::SDL
          ll::glib
          ll::gio
  )

endif ()
if( WINDOWS )
  target_link_libraries( ll::uilibraries INTERFACE
          opengl32
          comdlg32
          dxguid
          kernel32
          odbc32
          odbccp32
          oleaut32
          shell32
          Vfw32
          wer
          winspool
          imm32
          )
endif()

if (USE_FLATPAK)
target_include_directories( ll::uilibraries SYSTEM INTERFACE
        ${LIBS_PREBUILT_DIR}/include
        )
  pkg_check_modules(CAIRO-XLIB REQUIRED cairo-xlib)
  pkg_check_modules(DBUS-1 REQUIRED dbus-1)
  pkg_check_modules(LIBDECOR-0 REQUIRED libdecor-0)
  pkg_check_modules(PANGO REQUIRED pango)
  pkg_check_modules(PANGOCAIRO REQUIRED pangocairo)
  pkg_check_modules(WAYLAND-CLIENT REQUIRED wayland-client)
  pkg_check_modules(WAYLAND-CURSOR REQUIRED wayland-cursor)
  pkg_check_modules(XKBCOMMON REQUIRED xkbcommon)
  pkg_check_modules(XKBCOMMON-X11 REQUIRED xkbcommon-x11)
  target_link_libraries(ll::uilibraries INTERFACE
        ${CAIRO_LIBRARIES}
        ${CAIRO-XLIB_LIBRARIES}
        ${DBUS-1_LIBRARIES}
        ${LIBDECOR-0_LIBRARIES}
        ${PANGO_LIBRARIES}
        ${PANGOCAIRO_LIBRARIES}
        ${WAYLAND-CLIENT_LIBRARIES}
        ${WAYLAND-CURSOR_LIBRARIES}
        ${XKBCOMMON_LIBRARIES}
        ${XKBCOMMON-X11_LIBRARIES}
        )
endif ()
