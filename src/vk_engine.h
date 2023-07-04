#pragma once

#include <vector>

#include "vk_types.h"

class VulkanEngine {
   public:
    const uint32_t vk_version = VK_API_VERSION_1_3;

    void init();
    void cleanup();
    void draw();
    void run();

   private:
    size_t m_frame_number{0};

    bool m_is_initialized{false};

    VkExtent2D m_window_extent{1280, 720};
    struct SDL_Window *m_window{nullptr};
    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debug_messenger;
    VkSurfaceKHR m_surface;
    VkPhysicalDevice m_gpu;
    VkDevice m_device;
    VkSwapchainKHR m_swapchain;  // from other articles
    VkFormat m_swapchain_image_format;
    std::vector<VkImage> m_swapchain_images;
    std::vector<VkImageView> m_swapchain_image_views;
};
