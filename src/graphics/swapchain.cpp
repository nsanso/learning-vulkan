#include "swapchain.h"

#include <vulkan/vulkan.h>

#include <vector>

#include "utils.h"

void GraphicsSwapchain::destroy() {
    vkDestroyImageView(m_device, depth_view, nullptr);
    vmaDestroyImage(m_allocator, depth_image, m_depth_allocation);
    vkDestroySwapchainKHR(m_device, swapchain, nullptr);
    for (size_t i = 0; i < views.size(); i++) {
        vkDestroyImageView(m_device, views.at(i), nullptr);
    }
}

GraphicsSwapchainBuilder::GraphicsSwapchainBuilder(
    VkPhysicalDevice physical_device, VmaAllocator allocator, VkDevice device,
    VkSurfaceKHR surface)
    : m_physical_device(physical_device),
      m_device(device),
      m_surface(surface),
      m_extent(),
      capabilities(),
      formats(),
      present_modes(),
      create_info(VkSwapchainCreateInfoKHR{
          .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
          .surface = surface,
          .imageArrayLayers = 1,
          .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
          .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
          .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
          .presentMode = VK_PRESENT_MODE_FIFO_KHR,
          .clipped = VK_TRUE,
      }),
      m_allocator(allocator) {
    vk_check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        m_physical_device, m_surface, &capabilities));

    uint32_t format_count;
    vk_check(vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, m_surface,
                                                  &format_count, nullptr));
    formats.resize(format_count);
    vk_check(vkGetPhysicalDeviceSurfaceFormatsKHR(
        m_physical_device, m_surface, &format_count, formats.data()));

    uint32_t present_mode_count;
    vk_check(vkGetPhysicalDeviceSurfacePresentModesKHR(
        m_physical_device, m_surface, &present_mode_count, nullptr));
    present_modes.resize(present_mode_count);
    vk_check(vkGetPhysicalDeviceSurfacePresentModesKHR(
        m_physical_device, m_surface, &present_mode_count,
        present_modes.data()));

    create_info.minImageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount &&
        create_info.minImageCount > capabilities.maxImageCount) {
        create_info.minImageCount = capabilities.maxImageCount;
    }

    create_info.preTransform = capabilities.currentTransform;
}

GraphicsSwapchainBuilder* GraphicsSwapchainBuilder::set_extent(
    VkExtent2D extent) {
    m_extent = extent;
    return this;
}

GraphicsSwapchain GraphicsSwapchainBuilder::build() {
    GraphicsSwapchain destination{};
    destination.m_device = m_device;
    destination.m_allocator = m_allocator;

    destination.format = formats.at(0);
    destination.capabilities = capabilities;
    destination.present_modes = present_modes;

    create_info.imageExtent = m_extent;
    create_info.imageFormat = destination.format.format;
    create_info.imageColorSpace = destination.format.colorSpace;

    vk_check(vkCreateSwapchainKHR(m_device, &create_info, nullptr,
                                  &destination.swapchain));

    // Get swapchain images
    uint32_t swapchain_image_count;
    vkGetSwapchainImagesKHR(m_device, destination.swapchain,
                            &swapchain_image_count, nullptr);
    destination.images.resize(swapchain_image_count);
    vkGetSwapchainImagesKHR(m_device, destination.swapchain,
                            &swapchain_image_count, destination.images.data());

    // Get swapchain image views
    destination.views.resize(destination.images.size());
    for (size_t i = 0; i < destination.images.size(); i++) {
        VkImageViewCreateInfo view_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = destination.images.at(i),
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = destination.format.format,
            .subresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };
        vk_check(vkCreateImageView(m_device, &view_info, nullptr,
                                   &destination.views.at(i)));
    }

    // Get depth image
    destination.depth_format = VK_FORMAT_D32_SFLOAT;

    VkImageCreateInfo depth_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = destination.depth_format,
        .extent =
            VkExtent3D{
                .width = m_extent.width,
                .height = m_extent.height,
                .depth = 1,
            },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    };

    VmaAllocationCreateInfo depth_alloc_info{
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};

    vmaCreateImage(m_allocator, &depth_info, &depth_alloc_info,
                   &destination.depth_image, &destination.m_depth_allocation,
                   nullptr);

    // Get depth view
    VkImageViewCreateInfo depth_view_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = destination.depth_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = destination.depth_format,
        .subresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .levelCount = 1,
            .layerCount = 1,
        },
    };
    vk_check(vkCreateImageView(m_device, &depth_view_info, nullptr,
                               &destination.depth_view));

    return destination;
}
