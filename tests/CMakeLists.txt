find_package(GTest CONFIG REQUIRED)

add_executable(viewer_tests
    test_mesh.cpp
)

target_link_libraries(viewer_tests PRIVATE
    viewer_core
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(viewer_tests)
