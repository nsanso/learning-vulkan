#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <vector>

class GraphicsSwapchain {
   public:
    VkSwapchainKHR swapchain;
    VkSurfaceFormatKHR format;
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkPresentModeKHR> present_modes;
    std::vector<VkImage> images;
    std::vector<VkImageView> views;
    VkFormat depth_format;
    VkImage depth_image;
    VkImageView depth_view;

    void destroy();

   private:
    VkDevice m_device;
    VmaAllocator m_allocator;
    VmaAllocation m_depth_allocation;

    friend class GraphicsSwapchainBuilder;
};

class GraphicsSwapchainBuilder {
   public:
    GraphicsSwapchainBuilder(VkPhysicalDevice physical_device,
                             VmaAllocator allocator, VkDevice device,
                             VkSurfaceKHR surface);
    GraphicsSwapchainBuilder* set_extent(VkExtent2D extent);
    GraphicsSwapchain build();

    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;

   private:
    VkPhysicalDevice m_physical_device;
    VkDevice m_device;
    VkSurfaceKHR m_surface;
    VkExtent2D m_extent;
    VkSwapchainCreateInfoKHR create_info;
    VmaAllocator m_allocator;
};
