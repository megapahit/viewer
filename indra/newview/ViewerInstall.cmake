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
            "${ARCH_PREBUILT_DIRS_RELEASE}/libndofdev.dylib"
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

elseif (WINDOWS)

    install(DIRECTORY
        app_settings
        character
        fonts
        skins
        DESTINATION .
        )

    install(FILES
        ${AUTOBUILD_INSTALL_DIR}/ca-bundle.crt
        cube.dae
        featuretable.txt
        DESTINATION .
        )

    install(FILES
        licenses-win32.txt
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

    install(
        PROGRAMS
            ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${VIEWER_BINARY_NAME}.exe
            ${prefix_result}/../bin/OpenAL32.dll
            ${prefix_result}/../bin/alut.dll
            ${prefix_result}/../bin/boost_context-vc143-mt-x${ADDRESS_SIZE}-1_88.dll
            ${prefix_result}/../bin/boost_fiber-vc143-mt-x${ADDRESS_SIZE}-1_88.dll
            ${prefix_result}/../bin/boost_filesystem-vc143-mt-x${ADDRESS_SIZE}-1_88.dll
            ${prefix_result}/../bin/boost_json-vc143-mt-x${ADDRESS_SIZE}-1_88.dll
            ${prefix_result}/../bin/boost_program_options-vc143-mt-x${ADDRESS_SIZE}-1_88.dll
            ${prefix_result}/../bin/boost_thread-vc143-mt-x${ADDRESS_SIZE}-1_88.dll
            ${prefix_result}/../bin/boost_url-vc143-mt-x${ADDRESS_SIZE}-1_88.dll
            ${prefix_result}/../bin/brotlicommon.dll
            ${prefix_result}/../bin/brotlidec.dll
            ${prefix_result}/../bin/bz2.dll
            ${prefix_result}/../bin/fmt.dll
            ${prefix_result}/../bin/freetype.dll
            ${prefix_result}/../bin/hunspell-1.7-0.dll
            ${prefix_result}/../bin/iconv-2.dll
            ${prefix_result}/../bin/jpeg62.dll
            ${prefix_result}/../bin/libapr-1.dll
            ${prefix_result}/../bin/libaprutil-1.dll
            ${prefix_result}/../bin/libexpat.dll
            ${prefix_result}/../bin/libpng16.dll
            ${prefix_result}/../bin/libxml2.dll
            ${prefix_result}/../bin/meshoptimizer.dll
            ${prefix_result}/../bin/minizip.dll
            ${prefix_result}/../bin/nghttp2.dll
            ${prefix_result}/../bin/ogg.dll
            ${prefix_result}/../bin/openjp2.dll
            ${prefix_result}/../bin/vorbis.dll
            ${prefix_result}/../bin/vorbisenc.dll
            ${prefix_result}/../bin/vorbisfile.dll
            ${prefix_result}/../bin/zlib1.dll
        DESTINATION .
        )

    install(
        PROGRAMS
            ${prefix_result}/../bin/boost_context-vc143-mt-x${ADDRESS_SIZE}-1_88.dll
            ${prefix_result}/../bin/boost_fiber-vc143-mt-x${ADDRESS_SIZE}-1_88.dll
            ${prefix_result}/../bin/libapr-1.dll
            ${prefix_result}/../bin/libaprutil-1.dll
            ${prefix_result}/../bin/libexpat.dll
        DESTINATION llplugin
        )

else (DARWIN)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${VIEWER_BINARY_NAME}
        DESTINATION bin
        )

if (LINUX)
        if (${LINUX_DISTRO} MATCHES debian OR (${LINUX_DISTRO} MATCHES ubuntu))
                set(_LIB lib/${ARCH}-linux-gnu)
        elseif (${LINUX_DISTRO} MATCHES fedora OR (${LINUX_DISTRO} MATCHES opensuse-tumbleweed) OR (${LINUX_DISTRO} MATCHES gentoo))
                set(_LIB lib${ADDRESS_SIZE})
        else ()
                set(_LIB lib)
        endif ()
        if (USE_FMODSTUDIO)
            install(FILES
                ${ARCH_PREBUILT_DIRS_RELEASE}/libfmod.so
                ${ARCH_PREBUILT_DIRS_RELEASE}/libfmod.so.13
                ${ARCH_PREBUILT_DIRS_RELEASE}/libfmod.so.13.29
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
