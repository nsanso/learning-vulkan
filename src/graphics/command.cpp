#include "command.h"

#include <cassert>

GraphicsCommand GraphicsCommandBuilder::build() {
    GraphicsCommand out{};
    out.m_device = m_device;

    assert(!vkCreateCommandPool(m_device, &command_pool_info, nullptr,
                                &out.cmd_pool));

    cmdbuf_alloc_info.commandPool = out.cmd_pool;
    assert(
        !vkAllocateCommandBuffers(m_device, &cmdbuf_alloc_info, &out.cmd_buf));

    // Create Synchronization structures
    VkFenceCreateInfo fence_info{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    assert(!vkCreateFence(m_device, &fence_info, nullptr, &out.fence_render));

    VkSemaphoreCreateInfo semph_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    assert(
        !vkCreateSemaphore(m_device, &semph_info, nullptr, &out.semph_present));
    assert(
        !vkCreateSemaphore(m_device, &semph_info, nullptr, &out.semph_render));
    return out;
}

void GraphicsCommand::destroy() {
    vkDestroySemaphore(m_device, semph_render, nullptr);
    vkDestroySemaphore(m_device, semph_present, nullptr);
    vkDestroyFence(m_device, fence_render, nullptr);
    vkDestroyCommandPool(m_device, cmd_pool, nullptr);
}
