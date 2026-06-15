add_library(viewer_utils STATIC
    file_io.cpp
)

target_include_directories(viewer_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(viewer_utils PUBLIC
    spdlog::spdlog
)
