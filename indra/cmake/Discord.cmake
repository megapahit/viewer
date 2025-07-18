include(Prebuilt)

add_library(ll::discord INTERFACE IMPORTED)
target_compile_definitions(ll::discord INTERFACE LL_DISCORD=1)

if (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/discord_installed OR NOT ${discord_installed} EQUAL 0)
    file(ARCHIVE_EXTRACT
        INPUT $ENV{HOME}/Downloads/DiscordSocialSdk-1.4.9649.zip
        DESTINATION ${CMAKE_BINARY_DIR}
        )
    file(
        COPY
          ${CMAKE_BINARY_DIR}/discord_social_sdk/include/cdiscord.h
          ${CMAKE_BINARY_DIR}/discord_social_sdk/include/discordpp.h
        DESTINATION ${LIBS_PREBUILT_DIR}/include
        )
    if (WINDOWS)
        file(
            COPY ${CMAKE_BINARY_DIR}/discord_social_sdk/bin/release/discord_partner_sdk.dll
            DESTINATION ${LIBS_PREBUILT_DIR}/bin/release
            )
        set(LIBRARY_EXTENSION lib)
    else ()
        set(LIBRARY_PREFIX lib)
        set(LIBRARY_EXTENSION so)
    endif ()
    if (DARWIN)
        execute_process(
            COMMAND lipo
                libdiscord_partner_sdk.dylib
                -thin ${CMAKE_OSX_ARCHITECTURES}
                -output ${ARCH_PREBUILT_DIRS_RELEASE}/libdiscord_partner_sdk.dylib
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/discord_social_sdk/lib/release
            )
    else ()
        file(
            COPY ${CMAKE_BINARY_DIR}/discord_social_sdk/lib/release/${LIBRARY_PREFIX}discord_partner_sdk.${LIBRARY_EXTENSION}
            DESTINATION ${ARCH_PREBUILT_DIRS_RELEASE}
            )
    endif ()
    file(WRITE ${PREBUILD_TRACKING_DIR}/discord_installed "0")
endif ()

target_include_directories(ll::discord SYSTEM INTERFACE ${LIBS_PREBUILT_DIR}/include)
target_link_libraries(ll::discord INTERFACE discord_partner_sdk)
