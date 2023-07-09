#pragma once

#include <SDL.h>
#include <SDL_vulkan.h>
#include <VkBootstrap.h>

#include <cstdio>

class VulkanEngine {
   public:
    const uint32_t vk_version = VK_API_VERSION_1_3;

    void init();
    void cleanup();
    void draw();
    void run();

   private:
    size_t m_frame_count{0};

    bool m_initialized{false};

    VkExtent2D m_window_extent{1280, 720};
    struct SDL_Window *m_window{nullptr};

    vkb::Instance m_instance;
    VkSurfaceKHR m_surface;

    vkb::PhysicalDevice m_gpu;
    vkb::Device m_device;

    vkb::Swapchain m_swapchain;
    std::vector<VkImage> m_swapchain_images;
    std::vector<VkImageView> m_swapchain_views;

    VkQueue m_q_graphics;
    uint32_t m_qfamily_graphics;

    VkCommandPool m_cmd_pool;
    VkCommandBuffer m_cmd_buf;

    VkRenderPass m_render_pass;
    std::vector<VkFramebuffer> m_frame_bufs;

    VkSemaphore m_semph_present;
    VkSemaphore m_semph_render;
    VkFence m_fence_render;

    VkShaderModule m_vertex_shader;
    VkShaderModule m_fragment_shader;

    VkPipeline m_pipe;
    VkPipelineLayout m_pipelayout;

    bool load_shader(const uint32_t buffer[], size_t size, VkShaderModule *out);
};
