add_subdirectory(entt)

add_library(imgui STATIC
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    
    # We only need the GLFW and Vulkan backends!
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_vulkan.cpp
    
    ImGuizmo/ImGuizmo.cpp
)

target_include_directories(imgui PUBLIC 
    imgui 
    imgui/backends 
    ImGuizmo
)

target_link_libraries(imgui PUBLIC glfw Vulkan::Vulkan)