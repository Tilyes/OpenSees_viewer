cmake_minimum_required(VERSION 3.24)
project(OpenSeesViewer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    add_compile_options("/utf-8")
endif()

find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_subdirectory(src/core)
add_subdirectory(src/renderer)
add_subdirectory(src/adapter)
add_subdirectory(src/ui)
add_subdirectory(src/utils)

add_executable(opensees_viewer src/main.cpp)

target_link_libraries(opensees_viewer PRIVATE
    viewer_core
    viewer_renderer
    viewer_adapter
    viewer_ui
    viewer_utils
)

option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
