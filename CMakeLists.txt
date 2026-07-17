cmake_minimum_required(VERSION 3.16)

project(RevRenderEngine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
    message(STATUS "GLFW NOT FOUND. BUILDING FROM SOURCE")
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    add_subdirectory(external/glfw)
endif()

find_package(assimp QUIET)
if(NOT assimp_FOUND)
    message(STATUS "ASSIMP NOT FOUND. BUILDING FROM SOURCE")
    set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    add_subdirectory(external/assimp)
endif()

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/external/glm)

add_subdirectory(external)
add_subdirectory(engine)
add_subdirectory(backend)
add_subdirectory(revrender)

# Find the Slang compiler (assuming it's in your system PATH)
find_program(SLANGC_EXECUTABLE slangc)

if(SLANGC_EXECUTABLE)
    # Define the input and outputs
    set(SLANG_SOURCE "${CMAKE_SOURCE_DIR}/revrender/shaders/basic.slang")
    set(VERT_SPV "${CMAKE_SOURCE_DIR}/revrender/shaders/vert.spv")
    set(FRAG_SPV "${CMAKE_SOURCE_DIR}/revrender/shaders/frag.spv")

    # Tell CMake how to build the Vertex SPV
    add_custom_command(
        OUTPUT ${VERT_SPV}
        COMMAND ${SLANGC_EXECUTABLE} ${SLANG_SOURCE} -target spirv -profile glsl_460 -entry vertexMain -o ${VERT_SPV}
        DEPENDS ${SLANG_SOURCE}
    )

    # Tell CMake how to build the Fragment SPV
    add_custom_command(
        OUTPUT ${FRAG_SPV}
        COMMAND ${SLANGC_EXECUTABLE} ${SLANG_SOURCE} -target spirv -profile glsl_460 -entry fragmentMain -o ${FRAG_SPV}
        DEPENDS ${SLANG_SOURCE}
    )

    # Create a custom target that your engine depends on
    add_custom_target(CompileShaders ALL DEPENDS ${VERT_SPV} ${FRAG_SPV})
    
    # Ensure shaders compile BEFORE the engine builds
    add_dependencies(engine CompileShaders)
endif()