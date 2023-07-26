#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vk_mem_alloc.h>

#include <vector>

#include "pipeline.h"
#include "utils.h"

class GraphicsEngine {
   public:
    const uint32_t vk_version = VK_API_VERSION_1_3;

    GraphicsEngine();
    ~GraphicsEngine();
    void draw();
    void run();

   private:
    size_t m_frame_count{0};

    bool m_initialized{false};

    VkExtent2D m_window_extent{1280, 720};
    struct SDL_Window *m_window{nullptr};

    VkInstance m_instance;
    VkSurfaceKHR m_surface;

    VkPhysicalDevice m_gpu;
    VkDevice m_device;

    VkSwapchainKHR m_swapchain;
    VkSurfaceFormatKHR m_surface_format;
    std::vector<VkImage> m_swapchain_images;
    std::vector<VkImageView> m_swapchain_views;

    uint32_t m_qfamily_graphics{uint32_t(~0)};
    VkQueue m_q_graphics;

    VkCommandPool m_cmd_pool;
    VkCommandBuffer m_cmd_buf;

    VkRenderPass m_render_pass;
    std::vector<VkFramebuffer> m_frame_bufs;

    VkSemaphore m_semph_present;
    VkSemaphore m_semph_render;
    VkFence m_fence_render;

    GraphicsPipeline m_pipeline;

    VmaAllocator m_allocator;

    std::vector<Mesh> m_meshes;
};
