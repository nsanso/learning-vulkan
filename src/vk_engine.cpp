#include "vk_engine.h"

#include <SDL.h>
#include <SDL_vulkan.h>
#include <VkBootstrap.h>
#include <vulkan/vulkan_core.h>

#include <cstdio>
#include <iostream>

#include "vk_init.h"
#include "vk_types.h"

// Forward declarations
template <typename T>
const T &vkb_value_or_abort(vkb::Result<T> res);
VkResult vk_check(VkResult err);

// public
void VulkanEngine::init() {
    SDL_InitSubSystem(SDL_INIT_VIDEO);
    SDL_WindowFlags window_flags = (SDL_WINDOW_VULKAN);
    SDL_Window *window = SDL_CreateWindow(
        "Learning Vulkan", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        m_window_extent.width, m_window_extent.height, window_flags);

    vkb::Instance instance =
        vkb_value_or_abort(vkb::InstanceBuilder{}
                               .set_app_name("Learning Vulkan")
                               .request_validation_layers()
                               .require_api_version(vk_version)
                               .use_default_debug_messenger()
                               .build());

    VkSurfaceKHR surface;
    SDL_Vulkan_CreateSurface(window, instance, &surface);

    vkb::PhysicalDevice gpu = vkb_value_or_abort(
        vkb::PhysicalDeviceSelector{instance}
            .set_minimum_version(VK_API_VERSION_MAJOR(vk_version),
                                 VK_API_VERSION_MINOR(vk_version))
            .set_surface(surface)
            .select());

    vkb::Device device = vkb_value_or_abort(vkb::DeviceBuilder{gpu}.build());

    vkb::Swapchain swapchain = vkb_value_or_abort(
        vkb::SwapchainBuilder{gpu, device, surface}
            .use_default_format_selection()
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            .set_desired_extent(m_window_extent.width, m_window_extent.height)
            .build());

    m_window = window;
    m_instance = instance.instance;
    m_debug_messenger = instance.debug_messenger;
    m_surface = surface;
    m_gpu = gpu.physical_device;
    m_device = device;
    m_swapchain = swapchain.swapchain;
    m_swapchain_image_format = swapchain.image_format;
    m_swapchain_images = swapchain.get_images().value();
    m_swapchain_image_views = swapchain.get_image_views().value();

    m_is_initialized = true;
    std::cout << "VulkanEngine init_vulkan OK" << std::endl;
}

void VulkanEngine::cleanup() {
    if (!m_is_initialized) {
        printf("VulkanEngine::cleanup: not initialized\n");
        return;
    }

    // Destroy in the reverse order of creation
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    for (size_t i = 0; i < m_swapchain_image_views.size(); i++) {
        vkDestroyImageView(m_device, m_swapchain_image_views[i], nullptr);
    }
    vkDestroyDevice(m_device, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkb::destroy_debug_utils_messenger(m_instance, m_debug_messenger);
    vkDestroyInstance(m_instance, nullptr);
    SDL_DestroyWindow(m_window);
}

void VulkanEngine::run() {
    SDL_Event event;

    while (true) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                goto run_QUIT;
            }
        }
        draw();
    }

run_QUIT:
    return;
}

void VulkanEngine::draw() {}

VkResult vk_check(VkResult err) {
    if (err) {
        printf("Detected Vulkan error: %d\n", err);
        abort();
    }
    return err;
}

template <typename T>
const T &vkb_value_or_abort(vkb::Result<T> res) {
    if (!res.has_value()) {
        printf("Detected VKB error: %s\n",
               res.full_error().type.message().c_str());
        abort();
    }
    return res.value();
}
