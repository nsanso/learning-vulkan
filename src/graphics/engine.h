#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <vector>

#include "application.h"
#include "command.h"
#include "device.h"
#include "drawable.h"
#include "pipeline.h"
#include "render.h"
#include "swapchain.h"
#include "utils.h"

constexpr uint32_t FRAME_OVERLAP = 2;

class GraphicsEngine {
   public:
    const uint32_t vk_version = VK_API_VERSION_1_3;

    GraphicsEngine();
    ~GraphicsEngine();
    void draw();
    void run();

   private:
    size_t m_frame_count{0};

    VkExtent2D m_window_extent{1280, 720};
    struct SDL_Window* m_window{nullptr};

    VkSurfaceKHR m_surface;

    GraphicsApplication m_application;
    GraphicsDevice m_device;
    uint32_t m_qfamily_graphics{uint32_t(~0)};
    VkQueue m_q_graphics;

    VmaAllocator m_allocator;

    GraphicsSwapchain m_swapchain;
    GraphicsRender m_render;

    GraphicsCommand m_commands[FRAME_OVERLAP];

    std::vector<Drawable> m_drawables;
    std::vector<GraphicsPipeline> m_pipelines;
    std::vector<Mesh> m_meshes;

    GraphicsCommand* get_current_command();
};
