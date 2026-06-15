#include "vk_swapchain.h"
#include "vulkan/vk_device.h" // QueueFamilyIndices
#include <GLFW/glfw3.h>
#include <algorithm>
#include <spdlog/spdlog.h>

namespace viewer {

bool SwapchainCtx::init_phase1(VkPhysicalDevice physical_device, VkDevice device,
                               VkSurfaceKHR surface, GLFWwindow* window) {
    if (!create_swapchain(physical_device, device, surface, window)) return false;
    if (!create_image_views(device)) return false;
    return true;
}

bool SwapchainCtx::init_phase2(VkDevice device, VkRenderPass render_pass) {
    if (!create_framebuffers(device, render_pass)) return false;
    return true;
}

void SwapchainCtx::shutdown(VkDevice device) {
    for (auto& fb : framebuffers_) {
        vkDestroyFramebuffer(device, fb, nullptr);
    }
    framebuffers_.clear();

    for (auto& iv : image_views_) {
        vkDestroyImageView(device, iv, nullptr);
    }
    image_views_.clear();

    vkDestroySwapchainKHR(device, swapchain_, nullptr);
}

void SwapchainCtx::recreate(VkPhysicalDevice physical_device, VkDevice device,
                            VkSurfaceKHR surface, VkRenderPass render_pass,
                            GLFWwindow* window) {
    shutdown(device);
    init_phase1(physical_device, device, surface, window);
    init_phase2(device, render_pass);
    spdlog::info("Swapchain recreated: {}x{}", extent_.width, extent_.height);
}

bool SwapchainCtx::create_swapchain(VkPhysicalDevice physical_device, VkDevice device,
                                    VkSurfaceKHR surface, GLFWwindow* window) {
    // 1. 查询 GPU 表面能力
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);

    // 2. 查询支持的格式
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatCount, formats.data());

    // 3. 查询呈现模式
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentModeCount, presentModes.data());

    // --- 选最优配置 ---
    VkSurfaceFormatKHR surfaceFormat = formats[0];
    for (const auto& f : formats) {
        if (f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat = f;
            break;
        }
    }

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto& m : presentModes) {
        if (m == VK_PRESENT_MODE_MAILBOX_KHR) {
            presentMode = m;
            break;
        }
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    VkExtent2D extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
    extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    // 和 RenderLoop 的 semaphore 数量对齐，只用 2 张图
    uint32_t imageCount = capabilities.minImageCount;

    // --- 创建 Swapchain ---
    // 需要查询队列族来判断 sharing mode
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> qfs(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount, qfs.data());

    // 找 graphics 和 present 队列（简化版，不需要完整的 DeviceCtx）
    uint32_t graphicsIdx = 0, presentIdx = 0;
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (qfs[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) graphicsIdx = i;
        VkBool32 support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &support);
        if (support) presentIdx = i;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t indices[] = { graphicsIdx, presentIdx };
    if (graphicsIdx != presentIdx) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = indices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain_) != VK_SUCCESS) {
        spdlog::error("Failed to create swapchain");
        return false;
    }

    format_ = surfaceFormat.format;
    extent_ = extent;

    uint32_t actualCount = 0;
    vkGetSwapchainImagesKHR(device, swapchain_, &actualCount, nullptr);
    images_.resize(actualCount);
    vkGetSwapchainImagesKHR(device, swapchain_, &actualCount, images_.data());

    spdlog::info("Swapchain created: {}x{}, {} images", extent_.width, extent_.height, actualCount);
    return true;
}

bool SwapchainCtx::create_image_views(VkDevice device) {
    image_views_.resize(images_.size());

    for (size_t i = 0; i < images_.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = images_[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = format_;
        createInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                  VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &createInfo, nullptr, &image_views_[i]) != VK_SUCCESS) {
            spdlog::error("Failed to create image view {}", i);
            return false;
        }
    }
    return true;
}

bool SwapchainCtx::create_framebuffers(VkDevice device, VkRenderPass render_pass) {
    framebuffers_.resize(image_views_.size());

    for (size_t i = 0; i < image_views_.size(); i++) {
        VkImageView attachments[] = { image_views_[i] };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = render_pass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = extent_.width;
        framebufferInfo.height = extent_.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers_[i]) != VK_SUCCESS) {
            spdlog::error("Failed to create framebuffer {}", i);
            return false;
        }
    }
    return true;
}

} // namespace viewer
