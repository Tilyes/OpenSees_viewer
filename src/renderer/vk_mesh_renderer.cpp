#include "vk_mesh_renderer.h"
#include "vk_context.h"
#include <spdlog/spdlog.h>
#include <unordered_map>

namespace viewer {

bool VkMeshRenderer::init(VkContext& ctx) {
    // 1. 创建线管线（LINE_LIST，亮色彩虹）
    if (!pipeline_.init(ctx, "../shaders/line.vert.spv", "../shaders/line.frag.spv",
                        VK_PRIMITIVE_TOPOLOGY_LINE_LIST)) {
        spdlog::error("Failed to create mesh pipeline");
        return false;
    }

    // 2. 创建点管线（画节点）
    if (!point_pipeline_.init(ctx, "../shaders/mesh.vert.spv", "../shaders/mesh.frag.spv",
                              VK_PRIMITIVE_TOPOLOGY_POINT_LIST)) {
        spdlog::error("Failed to create point pipeline");
        return false;
    }

    // 3. 创建空 buffer 占位，直到 upload_mesh() 替换
    Vertex dummy;
    vertex_buffer_.create(ctx, sizeof(dummy),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vertex_buffer_.upload(ctx.device(), &dummy, sizeof(dummy));
    vertex_count_ = 0;

    point_buffer_.create(ctx, sizeof(dummy),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    point_buffer_.upload(ctx.device(), &dummy, sizeof(dummy));
    point_count_ = 0;

    // 4. UBO buffer：存 model/view/proj 三个矩阵的显存
    ubo_buffer_.create(ctx, sizeof(UniformBufferObject),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // 4. Descriptor Pool：分配 descriptor set 的内存池
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;

    if (vkCreateDescriptorPool(ctx.device(), &poolInfo, nullptr, &descriptor_pool_) != VK_SUCCESS) {
        spdlog::error("Failed to create descriptor pool");
        return false;
    }

    // 5. 从 pool 分配 descriptor set（按照 pipeline 的 layout）
    VkDescriptorSetLayout setLayout = pipeline_.descriptor_set_layout();
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptor_pool_;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &setLayout;

    if (vkAllocateDescriptorSets(ctx.device(), &allocInfo, &descriptor_set_) != VK_SUCCESS) {
        spdlog::error("Failed to allocate descriptor set");
        return false;
    }

    // 6. 把 descriptor set 的 binding=0 指向 UBO buffer
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = ubo_buffer_.handle();
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptor_set_;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(ctx.device(), 1, &descriptorWrite, 0, nullptr);

    // 保存 device 句柄，render 时需要
    device_ = ctx.device();
    initialized_ = true;
    return true;
}

void VkMeshRenderer::shutdown(VkContext& ctx) {
    vkDestroyDescriptorPool(ctx.device(), descriptor_pool_, nullptr);
    ubo_buffer_.destroy(ctx.device());
    vertex_buffer_.destroy(ctx.device());
    point_buffer_.destroy(ctx.device());
    pipeline_.shutdown(ctx.device());
    point_pipeline_.shutdown(ctx.device());
}

void VkMeshRenderer::upload_mesh(VkContext& ctx, const Mesh& mesh) {
    std::unordered_map<uint32_t, glm::vec3> node_map;
    std::vector<Vertex> line_vertices;
    std::vector<Vertex> point_vertices;

    // 填充节点 ID→坐标 查找表
    for (auto& n : mesh.nodes()) {
        node_map[n.id] = n.position;
        // 同时把节点坐标加入点列表，每个点 scalar 用序号/总数区分颜色
        float scalar = (float)point_vertices.size() / (float)mesh.nodes().size();
        point_vertices.push_back({ n.position, glm::vec3(0, 0, 1), scalar });
    }

    // 遍历 elements，把 Line2 单元展开为线段顶点
    for (auto& e : mesh.elements()) {
        if (e.type == ElementType::Line2 && e.node_ids.size() >= 2) {
            auto pos1 = node_map[e.node_ids[0]];
            auto pos2 = node_map[e.node_ids[1]];
            line_vertices.push_back({ pos1, glm::vec3(0, 0, 1), 0.5f });
            line_vertices.push_back({ pos2, glm::vec3(0, 0, 1), 0.5f });
        }
    }

    // 等 GPU 干完再销毁旧 buffer
    vkDeviceWaitIdle(ctx.device());

    // 上传线顶点
    vertex_buffer_.destroy(ctx.device());
    if (!line_vertices.empty()) {
        vertex_buffer_.create(ctx, line_vertices.size() * sizeof(Vertex),
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vertex_buffer_.upload(ctx.device(), line_vertices.data(), line_vertices.size() * sizeof(Vertex));
    }
    vertex_count_ = line_vertices.size();

    // 上传节点顶点
    point_buffer_.destroy(ctx.device());
    if (!point_vertices.empty()) {
        point_buffer_.create(ctx, point_vertices.size() * sizeof(Vertex),
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        point_buffer_.upload(ctx.device(), point_vertices.data(), point_vertices.size() * sizeof(Vertex));
    }
    point_count_ = point_vertices.size();
}

void VkMeshRenderer::render(VkCommandBuffer cmd, const VkExtent2D& extent, const UniformBufferObject& ubo) {
    if (!initialized_) return;

    //spdlog::info("Render: lines={} points={}", vertex_count_, point_count_);

    // 每帧上传最新的 UBO 数据（相机矩阵）到显存
    ubo_buffer_.upload(device_, &ubo, sizeof(UniformBufferObject));

    // 视口和裁剪
    VkViewport viewport{};
    viewport.x = 0.0f; viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f; viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = extent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    // 绑定顶点
    VkDeviceSize offset = 0;
    VkBuffer vkBuf = vertex_buffer_.handle();
    vkCmdBindVertexBuffers(cmd, 0, 1, &vkBuf, &offset);

    // 绑定 UBO descriptor set（点和线共用相同的 UBO）
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline_.layout(), 0, 1, &descriptor_set_, 0, nullptr);

    // ── 画线（LINE_LIST） ──
    if (vertex_count_ > 0) {
        VkBuffer lineBuf = vertex_buffer_.handle();
        vkCmdBindVertexBuffers(cmd, 0, 1, &lineBuf, &offset);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_.pipeline());
        vkCmdDraw(cmd, vertex_count_, 1, 0, 0);
    }

    // ── 画节点（POINT_LIST） ──
    if (point_count_ > 0) {
        VkBuffer ptBuf = point_buffer_.handle();
        vkCmdBindVertexBuffers(cmd, 0, 1, &ptBuf, &offset);
        // 重新绑定 descriptor set，用 point_pipeline 的 layout
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                point_pipeline_.layout(), 0, 1, &descriptor_set_, 0, nullptr);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, point_pipeline_.pipeline());
        vkCmdDraw(cmd, point_count_, 1, 0, 0);
    }
}

} // namespace viewer
