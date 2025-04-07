if (NOT ${viewer_VERSION})
  MESSAGE(FATAL_ERROR "Viewer version not known!")
endif (NOT ${viewer_VERSION})

set(INSTALL OFF CACHE BOOL
    "Generate install target.")

if (INSTALL)
  if (CMAKE_SYSTEM_NAME MATCHES FreeBSD)
      set(INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE PATH
          "Top-level installation directory.")
  else (CMAKE_SYSTEM_NAME MATCHES FreeBSD)
  set(INSTALL_PREFIX /usr CACHE PATH
      "Top-level installation directory.")
  endif (CMAKE_SYSTEM_NAME MATCHES FreeBSD)

  if (${LINUX_DISTRO} MATCHES debian OR (${LINUX_DISTRO} MATCHES ubuntu))
    set(_LIB lib/${ARCH}-linux-gnu)
  elseif (${LINUX_DISTRO} MATCHES fedora OR (${LINUX_DISTRO} MATCHES opensuse-tumbleweed) OR (${LINUX_DISTRO} MATCHES gentoo))
    set(_LIB lib${ADDRESS_SIZE})
  else ()
    set(_LIB lib)
  endif ()

  set(INSTALL_LIBRARY_DIR ${INSTALL_PREFIX}/${_LIB} CACHE PATH
      "Installation directory for read-only shared files.")

  set(INSTALL_SHARE_DIR ${INSTALL_PREFIX}/share CACHE PATH
      "Installation directory for read-only shared files.")

  set(APP_BINARY_DIR ${INSTALL_LIBRARY_DIR}/secondlife-${viewer_VERSION}
      CACHE PATH
      "Installation directory for binaries.")

  set(APP_SHARE_DIR ${INSTALL_SHARE_DIR}/${VIEWER_BINARY_NAME}
      CACHE PATH
      "Installation directory for read-only data files.")
  if (${LINUX_DISTRO} MATCHES arch)
    set(APP_LIBEXEC_DIR ${INSTALL_PREFIX}/lib/${VIEWER_BINARY_NAME}
        CACHE PATH
        "Installation directory for non-manual executables.")
  else (${LINUX_DISTRO} MATCHES arch)
    set(APP_LIBEXEC_DIR ${INSTALL_PREFIX}/libexec/${VIEWER_BINARY_NAME}
        CACHE PATH
        "Installation directory for non-manual executables.")
  endif (${LINUX_DISTRO} MATCHES arch)
endif (INSTALL)
