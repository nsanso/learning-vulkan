#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "swapchain.h"

class GraphicsRender {
   public:
    VkRenderPass renderpass;
    std::vector<VkFramebuffer> framebuffers;

    void destroy();

   private:
    VkDevice m_device;

    friend class GraphicsRenderBuilder;
};

class GraphicsRenderBuilder {
   public:
    GraphicsRenderBuilder(GraphicsSwapchain swapchain, VkDevice device);
    GraphicsRender build();

    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkAttachmentReference> references;
    std::vector<VkSubpassDependency> dependencies;
    VkSubpassDescription subpass_description;
    VkRenderPassCreateInfo render_pass_info;
    VkFramebufferCreateInfo framebuffer_info;

   private:
    GraphicsSwapchain m_swapchain;
    VkDevice m_device;
};
