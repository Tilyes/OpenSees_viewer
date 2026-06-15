add_library(viewer_renderer STATIC
    vulkan/vk_instance.cpp
    vulkan/vk_device.cpp
    vulkan/vk_swapchain.cpp
    vk_context.cpp
    vk_pipeline.cpp
    vk_buffer.cpp
    vk_mesh_renderer.cpp
    vk_imgui.cpp
    camera.cpp
    vk_render_loop.cpp
    vk_render_texture.cpp)

target_include_directories(viewer_renderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(viewer_renderer PUBLIC
    viewer_core
    viewer_utils
    Vulkan::Vulkan
    glfw
    glm::glm
    imgui::imgui
    spdlog::spdlog
)
