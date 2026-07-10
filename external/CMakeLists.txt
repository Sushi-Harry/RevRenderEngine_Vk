add_subdirectory(entt)

add_library(imgui STATIC
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_vulkan.cpp

    ImGuizmo/src/ImGuizmo.cpp
    ImGuizmo/src/ImCurveEdit.cpp
    ImGuizmo/src/ImGradient.cpp
    ImGuizmo/src/ImSequencer.cpp
    ImGuizmo/src/GraphEditor.cpp
)

add_subdirectory(ImGuizmo)

target_include_directories(imgui PUBLIC 
    imgui 
    imgui/backends 
    ImGuizmo
    ImGuizmo/src
)

target_link_libraries(imguizmo PUBLIC imgui)

target_link_libraries(imgui PUBLIC glfw Vulkan::Vulkan)