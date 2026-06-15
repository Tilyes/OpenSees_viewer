add_library(viewer_adapter STATIC
    file_source.cpp
    opensees_source.cpp
)

target_include_directories(viewer_adapter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(viewer_adapter PUBLIC
    viewer_core
    spdlog::spdlog
)
