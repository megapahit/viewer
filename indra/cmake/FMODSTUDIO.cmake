# -*- cmake -*-

include_guard()

# FMODSTUDIO can be set when launching the make using the argument -DUSE_FMODSTUDIO:BOOL=ON
# When building using proprietary binaries though (i.e. having access to LL private servers),
# we always build with FMODSTUDIO.
if (INSTALL_PROPRIETARY)
  set(USE_FMODSTUDIO ON CACHE BOOL "Using FMODSTUDIO sound library.")
endif (INSTALL_PROPRIETARY)

# ND: To streamline arguments passed, switch from FMODSTUDIO to USE_FMODSTUDIO
# To not break all old build scripts convert old arguments but warn about it
if(FMODSTUDIO)
  message( WARNING "Use of the FMODSTUDIO argument is deprecated, please switch to USE_FMODSTUDIO")
  set(USE_FMODSTUDIO ${FMODSTUDIO})
endif()

if (USE_FMODSTUDIO)
  add_library( ll::fmodstudio INTERFACE IMPORTED )
  target_compile_definitions( ll::fmodstudio INTERFACE LL_FMODSTUDIO=1)

  if (FMODSTUDIO_LIBRARY AND FMODSTUDIO_INCLUDE_DIR)
    # If the path have been specified in the arguments, use that

    target_link_libraries(ll::fmodstudio INTERFACE ${FMODSTUDIO_LIBRARY})
    target_include_directories( ll::fmodstudio SYSTEM INTERFACE  ${FMODSTUDIO_INCLUDE_DIR})
  else (FMODSTUDIO_LIBRARY AND FMODSTUDIO_INCLUDE_DIR)
    # If not, we're going to try to get the package listed in autobuild.xml
    # Note: if you're not using INSTALL_PROPRIETARY, the package URL should be local (file:/// URL)
    # as accessing the private LL location will fail if you don't have the credential
    include(Prebuilt)
    if (USESYSTEMLIBS AND (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/fmodstudio_installed OR NOT ${fmodstudio_installed} EQUAL 0))
      file(MAKE_DIRECTORY ${LIBS_PREBUILT_DIR}/lib/release)
      if (DARWIN)
        execute_process(
          COMMAND hdiutil attach -noverify fmodstudioapi20225mac-installer.dmg
          WORKING_DIRECTORY $ENV{HOME}/Downloads
          )
        file(
          COPY
            /Volumes/FMOD\ Programmers\ API\ Mac/FMOD\ Programmers\ API/api/core/inc/fmod.h
            /Volumes/FMOD\ Programmers\ API\ Mac/FMOD\ Programmers\ API/api/core/inc/fmod.hpp
            /Volumes/FMOD\ Programmers\ API\ Mac/FMOD\ Programmers\ API/api/core/inc/fmod_codec.h
            /Volumes/FMOD\ Programmers\ API\ Mac/FMOD\ Programmers\ API/api/core/inc/fmod_common.h
            /Volumes/FMOD\ Programmers\ API\ Mac/FMOD\ Programmers\ API/api/core/inc/fmod_dsp.h
            /Volumes/FMOD\ Programmers\ API\ Mac/FMOD\ Programmers\ API/api/core/inc/fmod_dsp_effects.h
            /Volumes/FMOD\ Programmers\ API\ Mac/FMOD\ Programmers\ API/api/core/inc/fmod_errors.h
            /Volumes/FMOD\ Programmers\ API\ Mac/FMOD\ Programmers\ API/api/core/inc/fmod_output.h
          DESTINATION ${LIBS_PREBUILT_DIR}/include/fmodstudio
          )
        execute_process(
          COMMAND lipo
            lib/libfmod.dylib
            -thin ${CMAKE_OSX_ARCHITECTURES}
            -output ${LIBS_PREBUILT_DIR}/lib/release/libfmod.dylib
          WORKING_DIRECTORY /Volumes/FMOD\ Programmers\ API\ Mac/FMOD\ Programmers\ API/api/core
          )
        execute_process(
          COMMAND hdiutil detach FMOD\ Programmers\ API\ Mac
          WORKING_DIRECTORY /Volumes
          RESULT_VARIABLE fmodstudio_installed
          )
        file(WRITE ${PREBUILD_TRACKING_DIR}/fmodstudio_installed "${fmodstudio_installed}")
      else (DARWIN)
        file(ARCHIVE_EXTRACT
          INPUT $ENV{HOME}/Downloads/fmodstudioapi20225linux.tar.gz
          DESTINATION ${CMAKE_BINARY_DIR}
          )
        file(
          COPY
            ${CMAKE_BINARY_DIR}/fmodstudioapi20225linux/api/core/inc/fmod.h
            ${CMAKE_BINARY_DIR}/fmodstudioapi20225linux/api/core/inc/fmod.hpp
            ${CMAKE_BINARY_DIR}/fmodstudioapi20225linux/api/core/inc/fmod_codec.h
            ${CMAKE_BINARY_DIR}/fmodstudioapi20225linux/api/core/inc/fmod_common.h
            ${CMAKE_BINARY_DIR}/fmodstudioapi20225linux/api/core/inc/fmod_dsp.h
            ${CMAKE_BINARY_DIR}/fmodstudioapi20225linux/api/core/inc/fmod_dsp_effects.h
            ${CMAKE_BINARY_DIR}/fmodstudioapi20225linux/api/core/inc/fmod_errors.h
            ${CMAKE_BINARY_DIR}/fmodstudioapi20225linux/api/core/inc/fmod_output.h
          DESTINATION ${LIBS_PREBUILT_DIR}/include/fmodstudio
          )
        file(
          COPY
            ${CMAKE_BINARY_DIR}/fmodstudioapi20225linux/api/core/lib/${CMAKE_SYSTEM_PROCESSOR}/libfmod.so
            ${CMAKE_BINARY_DIR}/fmodstudioapi20225linux/api/core/lib/${CMAKE_SYSTEM_PROCESSOR}/libfmod.so.13
            ${CMAKE_BINARY_DIR}/fmodstudioapi20225linux/api/core/lib/${CMAKE_SYSTEM_PROCESSOR}/libfmod.so.13.25
          DESTINATION ${LIBS_PREBUILT_DIR}/lib/release
          FOLLOW_SYMLINK_CHAIN
          )
        file(WRITE ${PREBUILD_TRACKING_DIR}/fmodstudio_installed "0")
      endif (DARWIN)
    else (USESYSTEMLIBS AND (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/fmodstudio_installed OR NOT ${fmodstudio_installed} EQUAL 0))
    use_prebuilt_binary(fmodstudio)
    endif (USESYSTEMLIBS AND (${PREBUILD_TRACKING_DIR}/sentinel_installed IS_NEWER_THAN ${PREBUILD_TRACKING_DIR}/fmodstudio_installed OR NOT ${fmodstudio_installed} EQUAL 0))
    if (WINDOWS)
      target_link_libraries( ll::fmodstudio INTERFACE  fmod_vc)
    elseif (DARWIN)
      #despite files being called libfmod.dylib, we are searching for fmod
      target_link_libraries( ll::fmodstudio INTERFACE  fmod)
    elseif (LINUX)
      target_link_libraries( ll::fmodstudio INTERFACE  fmod)
    endif (WINDOWS)

    target_include_directories( ll::fmodstudio SYSTEM INTERFACE ${LIBS_PREBUILT_DIR}/include/fmodstudio)
  endif (FMODSTUDIO_LIBRARY AND FMODSTUDIO_INCLUDE_DIR)
else()
  set( USE_FMODSTUDIO "OFF")
endif ()

