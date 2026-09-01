# -*- cmake -*-

project(llmeshoptimizer)

include(Meshoptimizer)

include(00-Common)
include(LLCommon)
include(LLMath)

set(llmeshoptimizer_SOURCE_FILES
        llmeshoptimizer.cpp
        )

set(llmeshoptimizer_HEADER_FILES
        CMakeLists.txt
        llmeshoptimizer.h
        )

set_source_files_properties(${llmeshoptimizer_HEADER_FILES}
        PROPERTIES HEADER_FILE_ONLY TRUE)

list(APPEND llmeshoptimizer_SOURCE_FILES ${llmeshoptimizer_HEADER_FILES})

add_library (llmeshoptimizer ${llmeshoptimizer_SOURCE_FILES})
target_include_directories( llmeshoptimizer INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${CMAKE_SOURCE_DIR}/llmath)
target_link_libraries(llmeshoptimizer
        llcommon
        ll::meshoptimizer)

include(LibraryInstall)

if (USE_PRECOMPILED_HEADERS)
  target_precompile_headers(llmeshoptimizer REUSE_FROM llprecompiled)
endif ()

# Add tests

