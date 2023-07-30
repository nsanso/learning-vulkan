#include "swapchain.h"

#include <vulkan/vulkan.h>

#include <vector>

#include "utils.h"

void GraphicsSwapchain::destroy() {
    vkDestroySwapchainKHR(m_device, swapchain, nullptr);
    for (size_t i = 0; i < views.size(); i++) {
        vkDestroyImageView(m_device, views.at(i), nullptr);
    }
}

GraphicsSwapchainBuilder::GraphicsSwapchainBuilder(
    VkPhysicalDevice physical_device, VkDevice device, VkSurfaceKHR surface)
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
      }) {
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

    return destination;
}
