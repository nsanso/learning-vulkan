#include "render.h"

#include "utils.h"

void GraphicsRender::destroy() {
    vkDestroyRenderPass(m_device, renderpass, nullptr);
    for (auto framebuffer : framebuffers) {
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }
}

GraphicsRenderBuilder::GraphicsRenderBuilder(GraphicsSwapchain swapchain,
                                             VkDevice device)
    : attachments({
          VkAttachmentDescription{
              .format = swapchain.format.format,
              .samples = VK_SAMPLE_COUNT_1_BIT,
              .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
              .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
              .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
              .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
              .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
              .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
          },
          VkAttachmentDescription{
              .format = swapchain.depth_format,
              .samples = VK_SAMPLE_COUNT_1_BIT,
              .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
              .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
              .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
              .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
              .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
              .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
          },
      }),

      references({
          VkAttachmentReference{
              .attachment = 0,
              .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
          },
          VkAttachmentReference{
              .attachment = 1,
              .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
          },
      }),

      dependencies({
          VkSubpassDependency{
              .srcSubpass = VK_SUBPASS_EXTERNAL,
              .dstSubpass = 0,
              .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
              .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
              .srcAccessMask = 0,
              .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
          },
          VkSubpassDependency{
              .srcSubpass = VK_SUBPASS_EXTERNAL,
              .dstSubpass = 0,
              .srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                              VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
              .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                              VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
              .srcAccessMask = 0,
              .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
          },
      }),

      subpass_description(VkSubpassDescription{
          .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
          .colorAttachmentCount = 1,
          .pColorAttachments = &references.at(0),
          .pDepthStencilAttachment = &references.at(1),
      }),

      render_pass_info(VkRenderPassCreateInfo{
          .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
          .attachmentCount = (uint32_t)attachments.size(),
          .pAttachments = attachments.data(),
          .subpassCount = 1,
          .pSubpasses = &subpass_description,
          .dependencyCount = (uint32_t)dependencies.size(),
          .pDependencies = dependencies.data(),
      }),

      framebuffer_info(VkFramebufferCreateInfo{
          .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
          .pNext = nullptr,
          .width = swapchain.extent.width,
          .height = swapchain.extent.height,
          .layers = 1,
      }),

      m_swapchain(swapchain),

      m_device(device) {}

GraphicsRender GraphicsRenderBuilder::build() {
    GraphicsRender out{};
    out.m_device = m_device;

    vk_check(vkCreateRenderPass(m_device, &render_pass_info, nullptr,
                                &out.renderpass));
    framebuffer_info.renderPass = out.renderpass;

    out.framebuffers.resize(m_swapchain.images.size());
    for (size_t i = 0; i < m_swapchain.images.size(); i++) {
        VkImageView attachments[]{m_swapchain.views[i], m_swapchain.depth_view};

        framebuffer_info.attachmentCount =
            sizeof(attachments) / sizeof(attachments[0]);
        framebuffer_info.pAttachments = attachments;

        vk_check(vkCreateFramebuffer(m_device, &framebuffer_info, nullptr,
                                     &out.framebuffers[i]));
    }

    return out;
}
