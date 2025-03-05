# -*- cmake -*-

project(llprimitive)

include(00-Common)
include(Prebuilt)
include(LLCommon)
include(LLCoreHttp)
include(LLPhysicsExtensions)
include(LLPrimitive)
include(GLM)
include(TinyGLTF)

if(LINUX OR CMAKE_SYSTEM_NAME MATCHES FreeBSD )
  set_property(DIRECTORY APPEND
    PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/../patches/collada-dom-v2.3-r8.patch)
  prepare_thirdparty(
    "https://github.com/secondlife/3p-colladadom/archive/refs/tags/v2.3-r8.tar.gz"
    "colladadom-v2.3-r8.tar.gz"
    "3p-colladadom-2.3-r8"
    "collada-dom-v2.3-r8.patch"
    "b36dce3b7be4b49f3c8e4d6c9dbf3529f7ed1f28e64ee45be0e77805f06376b95079798616529302d771ab2b76bf06d2f894ca11e6f7f3097c3f7a05a78c474f"
  )
  set(OPT_COLLADA14 TRUE)
  add_subdirectory(
	  "${CMAKE_SOURCE_DIR}/../../3p-colladadom-2.3-r8" "colladadom")
  target_include_directories(
	  ll::colladadom INTERFACE
	  ${colladadom_SOURCE_DIR}/include
	  ${colladadom_SOURCE_DIR}/include/1.4
  )
  target_link_libraries( ll::colladadom INTERFACE collada14dom )
endif(LINUX OR CMAKE_SYSTEM_NAME MATCHES FreeBSD )

set(llprimitive_SOURCE_FILES
    lldaeloader.cpp
    llgltfloader.cpp
    llgltfmaterial.cpp
    llmaterialid.cpp
    llmaterial.cpp
    llmaterialtable.cpp
    llmediaentry.cpp
    llmodel.cpp
    llmodelloader.cpp
    llprimitive.cpp
    llprimtexturelist.cpp
    lltextureanim.cpp
    lltextureentry.cpp
    lltreeparams.cpp
    llvolumemessage.cpp
    material_codes.cpp
    )

set(llprimitive_HEADER_FILES
    CMakeLists.txt
    lldaeloader.h
    llgltfloader.h
    llgltfmaterial.h
    llgltfmaterial_templates.h
    legacy_object_types.h
    llmaterial.h
    llmaterialid.h
    llmaterialtable.h
    llmediaentry.h
    llmodel.h
    llmodelloader.h
    llprimitive.h
    llprimtexturelist.h
    lllslconstants.h
    lltextureanim.h
    lltextureentry.h
    lltreeparams.h
    lltree_common.h
    llvolumemessage.h
    material_codes.h
    object_flags.h
    )

list(APPEND llprimitive_SOURCE_FILES ${llprimitive_HEADER_FILES})

add_library (llprimitive ${llprimitive_SOURCE_FILES})
target_include_directories( llprimitive  INTERFACE   ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(llprimitive
        llcommon
        llmath
        llmessage
        llcorehttp
        llxml
        llcharacter
        llrender
        llphysicsextensions_impl
        ll::colladadom
        ll::glm
        )

include(LibraryInstall)

#add unit tests
if (LL_TESTS)
    INCLUDE(LLAddBuildTest)
    SET(llprimitive_TEST_SOURCE_FILES
      llmediaentry.cpp
      llprimitive.cpp
      llgltfmaterial.cpp
      )

    set_property(SOURCE llprimitive.cpp PROPERTY LL_TEST_ADDITIONAL_LIBRARIES llmessage)
    LL_ADD_PROJECT_UNIT_TESTS(llprimitive "${llprimitive_TEST_SOURCE_FILES}")
endif (LL_TESTS)
