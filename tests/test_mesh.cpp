#include <gtest/gtest.h>
#include "mesh.h"

TEST(MeshTest, AddNodeAndElement) {
    viewer::Mesh mesh;
    EXPECT_TRUE(mesh.empty());

    mesh.add_node(1, {0.0f, 0.0f, 0.0f});
    mesh.add_node(2, {1.0f, 0.0f, 0.0f});
    mesh.add_node(3, {0.0f, 1.0f, 0.0f});

    EXPECT_EQ(mesh.nodes().size(), 3);

    mesh.add_element(1, viewer::ElementType::Tri3, {1, 2, 3});
    EXPECT_EQ(mesh.elements().size(), 1);
    EXPECT_EQ(mesh.elements()[0].node_ids.size(), 3);
}

TEST(MeshTest, Clear) {
    viewer::Mesh mesh;
    mesh.add_node(1, {0.0f, 0.0f, 0.0f});
    mesh.clear();
    EXPECT_TRUE(mesh.empty());
}
