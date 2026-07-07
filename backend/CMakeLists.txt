
file(GLOB_RECURSE BACKEND_SOURCES "src/*.cpp")

add_library(graphic_backend STATIC
    ${BACKEND_SOURCES}
)

target_include_directories(graphic_backend PUBLIC
    include
    "${CMAKE_CURRENT_SOURCE_DIR}/../external"
)

target_link_libraries(graphic_backend PUBLIC engine_core glm Vulkan::Vulkan imgui)

if(TARGET glfw)
    target_link_libraries(graphic_backend PUBLIC glfw)
else()
    target_link_libraries(graphic_backend PUBLIC glfw::glfw)
endif()