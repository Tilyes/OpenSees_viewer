add_library(viewer_core STATIC
    mesh.cpp
    field.cpp
    model.cpp
    element_registry.cpp
)

target_include_directories(viewer_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

find_package(glm CONFIG REQUIRED)
target_link_libraries(viewer_core PUBLIC glm::glm)
