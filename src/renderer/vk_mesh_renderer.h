#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "model.h"
#include "vk_pipeline.h"
#include "vk_buffer.h"

namespace viewer {

class VkContext;

struct Vertex {
    glm::vec3 pos;      // location=0
    glm::vec3 normal;   // location=1
    float scalar;       // location=2
};

// 和 mesh.vert 里 layout(binding=0) 的 UBO 完全对齐
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

class VkMeshRenderer {
public:
    bool init(VkContext& ctx);
    void shutdown(VkContext& ctx);
    void upload_mesh(VkContext& ctx, const Mesh& mesh);

    // 每帧渲染：cmd = 命令缓冲区, extent = 窗口尺寸, ubo = 相机矩阵
    void render(VkCommandBuffer cmd, const VkExtent2D& extent, const UniformBufferObject& ubo);

private:
    Pipeline pipeline_;          // 线框管线 (LINE_LIST)
    Pipeline point_pipeline_;    // 节点管线 (POINT_LIST)
    GpuBuffer vertex_buffer_;    // 线顶点
    GpuBuffer point_buffer_;     // 节点顶点
    uint32_t vertex_count_ = 0;
    uint32_t point_count_ = 0;
    bool initialized_ = false;

    // UBO 相关
    GpuBuffer ubo_buffer_;
    VkDescriptorPool descriptor_pool_ = VK_NULL_HANDLE;
    VkDescriptorSet descriptor_set_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
};

} // namespace viewer
