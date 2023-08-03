#pragma once

#include <vulkan/vulkan_core.h>

class GraphicsCommand {
   public:
    VkCommandPool cmd_pool;
    VkCommandBuffer cmd_buf;

    VkSemaphore semph_present;
    VkSemaphore semph_render;
    VkFence fence_render;

    void destroy();

   private:
    VkDevice m_device;

    friend class GraphicsCommandBuilder;
};

class GraphicsCommandBuilder {
   public:
    GraphicsCommandBuilder(VkDevice device, uint32_t queue_family)
        : command_pool_info(VkCommandPoolCreateInfo{
              .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
              .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
              .queueFamilyIndex = queue_family,
          }),

          cmdbuf_alloc_info(VkCommandBufferAllocateInfo{
              .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
              .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
              .commandBufferCount = 1,
          }),

          fence_info(VkFenceCreateInfo{
              .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
              .flags = VK_FENCE_CREATE_SIGNALED_BIT,
          }),

          semph_info(VkSemaphoreCreateInfo{
              .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
          }),

          m_device(device){};

    GraphicsCommand build();

   private:
    VkCommandPoolCreateInfo command_pool_info;
    VkCommandBufferAllocateInfo cmdbuf_alloc_info;
    VkFenceCreateInfo fence_info;
    VkSemaphoreCreateInfo semph_info;
    VkDevice m_device;
};
