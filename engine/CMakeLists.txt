file(GLOB_RECURSE ENGINE_SOURCES "./*.cpp")

add_library(engine_core STATIC ${ENGINE_SOURCES})

target_include_directories(engine_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

target_link_libraries(engine_core PUBLIC glm imgui imguizmo::imguizmo assimp)