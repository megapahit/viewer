include(Linking)
include(Prebuilt)

include_guard()

add_library( ll::apr INTERFACE IMPORTED )

if (WINDOWS)
  target_include_directories(ll::apr SYSTEM INTERFACE ${prefix_result}/../include)
  target_link_directories(ll::apr INTERFACE ${prefix_result})
  target_link_libraries(ll::apr INTERFACE libapr-1 libaprutil-1)
else ()
  include(FindPkgConfig)
  pkg_check_modules(Apr REQUIRED apr-1 apr-util-1)
  target_include_directories(ll::apr SYSTEM INTERFACE ${Apr_INCLUDE_DIRS})
  target_link_directories(ll::apr INTERFACE ${Apr_LIBRARY_DIRS})
  target_link_libraries(ll::apr INTERFACE ${Apr_LIBRARIES})
endif ()

return ()

use_system_binary( apr apr-util )
use_prebuilt_binary(apr_suite)

if (WINDOWS)
  if (LLCOMMON_LINK_SHARED)
    set(APR_selector "lib")
  else (LLCOMMON_LINK_SHARED)
    set(APR_selector "")
  endif (LLCOMMON_LINK_SHARED)
  target_link_libraries( ll::apr INTERFACE
          ${ARCH_PREBUILT_DIRS_RELEASE}/${APR_selector}apr-1.lib
          ${ARCH_PREBUILT_DIRS_RELEASE}/${APR_selector}aprutil-1.lib
          )
  target_compile_definitions( ll::apr INTERFACE APR_DECLARE_STATIC=1 APU_DECLARE_STATIC=1 API_DECLARE_STATIC=1)
elseif (DARWIN)
  if (LLCOMMON_LINK_SHARED)
    set(APR_selector     "0.dylib")
    set(APRUTIL_selector "0.dylib")
  else (LLCOMMON_LINK_SHARED)
    set(APR_selector     "a")
    set(APRUTIL_selector "a")
  endif (LLCOMMON_LINK_SHARED)

  target_link_libraries( ll::apr INTERFACE
          ${ARCH_PREBUILT_DIRS_RELEASE}/libapr-1.${APR_selector}
          ${ARCH_PREBUILT_DIRS_RELEASE}/libaprutil-1.${APR_selector}
          iconv
          )
else()
  target_link_libraries( ll::apr INTERFACE
          ${ARCH_PREBUILT_DIRS_RELEASE}/libapr-1.a
          ${ARCH_PREBUILT_DIRS_RELEASE}/libaprutil-1.a
          rt
          )
endif ()
target_include_directories( ll::apr SYSTEM INTERFACE  ${LIBS_PREBUILT_DIR}/include/apr-1 )
