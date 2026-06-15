add_library(viewer_ui STATIC
    app.cpp
    viewport.cpp
    inspector.cpp
    timeline.cpp
)

target_include_directories(viewer_ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(viewer_ui PUBLIC
    viewer_core
    viewer_renderer
    viewer_adapter
    imgui::imgui
    spdlog::spdlog
)
