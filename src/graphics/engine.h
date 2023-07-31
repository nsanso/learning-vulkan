#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <vector>

#include "application.h"
#include "device.h"
#include "pipeline.h"
#include "render.h"
#include "swapchain.h"
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

    VkSurfaceKHR m_surface;

    GraphicsApplication m_application;
    GraphicsDevice m_device;

    GraphicsSwapchain m_swapchain;

    uint32_t m_qfamily_graphics{uint32_t(~0)};
    VkQueue m_q_graphics;

    VkCommandPool m_cmd_pool;
    VkCommandBuffer m_cmd_buf;

    GraphicsRender m_render;

    VkSemaphore m_semph_present;
    VkSemaphore m_semph_render;
    VkFence m_fence_render;

    GraphicsPipeline m_pipeline;

    VmaAllocator m_allocator;

    std::vector<Mesh> m_meshes;
};
