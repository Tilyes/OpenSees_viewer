#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace viewer {

class VkContext;

class Pipeline {
public:
    bool init(VkContext& ctx, const std::string& vert_path, const std::string& frag_path,
              VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    void shutdown(VkDevice device);

    VkPipelineLayout layout() const { return layout_; }
    VkPipeline pipeline() const { return pipeline_; }
    VkDescriptorSetLayout descriptor_set_layout() const { return descriptor_set_layout_; }

private:
    VkShaderModule create_shader_module(VkDevice device, const std::vector<char>& code);

    VkDescriptorSetLayout descriptor_set_layout_ = VK_NULL_HANDLE;
    VkPipelineLayout layout_ = VK_NULL_HANDLE;
    VkPipeline pipeline_ = VK_NULL_HANDLE;
};

} // namespace viewer
