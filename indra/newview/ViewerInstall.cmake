if (DARWIN)

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/InfoPlist.strings
        ${CMAKE_CURRENT_SOURCE_DIR}/English.lproj/language.txt
        DESTINATION English.lproj
        )

    install(DIRECTORY
        German.lproj
        Japanese.lproj
        Korean.lproj
        app_settings
        character
        cursors_mac
        da.lproj
        es.lproj
        fonts
        fr.lproj
        uk.lproj
        hu.lproj
        it.lproj
        nl.lproj
        pl.lproj
        pt.lproj
        ru.lproj
        skins
        tr.lproj
        zh-Hans.lproj
        DESTINATION .
        )

    install(FILES
        SecondLife.nib
        ${AUTOBUILD_INSTALL_DIR}/ca-bundle.crt
        cube.dae
        featuretable_mac.txt
        DESTINATION .
        )

    if (NOT PACKAGE)
        install(FILES
            secondlife.icns
            RENAME ${VIEWER_CHANNEL}.icns
            DESTINATION .
            )
    endif (NOT PACKAGE)

    install(FILES
        licenses-mac.txt
        RENAME licenses.txt
        DESTINATION .
        )

    install(FILES
        ${SCRIPTS_DIR}/messages/message_template.msg
        ${SCRIPTS_DIR}/../etc/message.xml
        ${CMAKE_CURRENT_BINARY_DIR}/contributors.txt
        DESTINATION app_settings
        )

    install(DIRECTORY
        ${AUTOBUILD_INSTALL_DIR}/dictionaries
        DESTINATION app_settings
        )

    if (NDOF)
        install(FILES
            "${AUTOBUILD_INSTALL_DIR}/lib/release/libndofdev.dylib"
            DESTINATION .
            )
    endif ()

    if (PACKAGE)
        configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/FixPackage.cmake.in
            ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake
            )
    else (PACKAGE)
        configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/FixBundle.cmake.in
            ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake
            )
    endif (PACKAGE)
    install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake)

else (DARWIN)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${VIEWER_BINARY_NAME}
        DESTINATION bin
        )

install(PROGRAMS linux_tools/launch_url.sh
        DESTINATION libexec/${VIEWER_BINARY_NAME}
        )

if (LINUX)
        if (EXISTS ${CMAKE_SYSROOT}/usr/lib/${ARCH}-linux-gnu)
                set(_LIB lib/${ARCH}-linux-gnu)
        elseif (EXISTS /lib64 AND NOT ${LINUX_DISTRO} MATCHES arch)
                set(_LIB lib64)
        else ()
                set(_LIB lib)
        endif ()
        if (USE_FMODSTUDIO)
            install(FILES
                ${AUTOBUILD_INSTALL_DIR}/lib/release/libfmod.so
                ${AUTOBUILD_INSTALL_DIR}/lib/release/libfmod.so.13
                ${AUTOBUILD_INSTALL_DIR}/lib/release/libfmod.so.13.25
            DESTINATION ${_LIB})
        endif (USE_FMODSTUDIO)
endif (LINUX)

install(DIRECTORY skins app_settings fonts
        DESTINATION share/${VIEWER_BINARY_NAME}
        PATTERN ".svn" EXCLUDE
        )

install(DIRECTORY icons/hicolor
        DESTINATION share/icons
        )

find_file(IS_ARTWORK_PRESENT NAMES have_artwork_bundle.marker
          PATHS ${VIEWER_DIR}/newview/res)

if (IS_ARTWORK_PRESENT)
  install(DIRECTORY res res-sdl character
          DESTINATION share/${VIEWER_BINARY_NAME}
          PATTERN ".svn" EXCLUDE
          )
else (IS_ARTWORK_PRESENT)
  message(STATUS "WARNING: Artwork is not present, and will not be installed")
endif (IS_ARTWORK_PRESENT)

    install(FILES
        ${AUTOBUILD_INSTALL_DIR}/ca-bundle.crt
        featuretable_linux.txt
        #featuretable_solaris.txt
        DESTINATION share/${VIEWER_BINARY_NAME}
        )

    install(FILES
        licenses-linux.txt
        RENAME licenses.txt
        DESTINATION share/${VIEWER_BINARY_NAME}
        )

install(FILES ${SCRIPTS_DIR}/messages/message_template.msg
        ${SCRIPTS_DIR}/../etc/message.xml
        ${CMAKE_CURRENT_BINARY_DIR}/contributors.txt
        DESTINATION share/${VIEWER_BINARY_NAME}/app_settings
        )

    install(DIRECTORY
        ${AUTOBUILD_INSTALL_DIR}/dictionaries
        DESTINATION share/${VIEWER_BINARY_NAME}/app_settings
        )

    install(FILES linux_tools/${VIEWER_BINARY_NAME}.desktop
        DESTINATION share/applications
        )

endif (DARWIN)
