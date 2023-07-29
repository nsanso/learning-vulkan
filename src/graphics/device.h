#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

struct GraphicsQueue {
    uint32_t family;
    VkQueue handle;
};

class GraphicsDevice {
   public:
    VkDevice device;
    std::vector<GraphicsQueue> queues;

    VkQueue get_queue(uint32_t family);
    void destroy();
};

class GraphicsDeviceBuilder {
   public:
    GraphicsDevice build();

    GraphicsDeviceBuilder *add_device_extension(const char *layer);
    GraphicsDeviceBuilder *add_queue(uint32_t family, float priority);

    GraphicsDeviceBuilder(VkPhysicalDevice physical_device)
        : physical_device(physical_device),
          features(VkPhysicalDeviceFeatures{}),
          device_extensions(std::vector<const char *>{
              VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#ifndef NDEBUG
              VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
#endif
          }),
          queue_infos(std::vector<VkDeviceQueueCreateInfo>{}),
          queue_priorities(std::vector<float>{}),
          device_info(VkDeviceCreateInfo{
              .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
              .pEnabledFeatures = &features,
          }) {
    }

   private:
    VkPhysicalDevice physical_device;
    VkPhysicalDeviceFeatures features;
    std::vector<const char *> device_extensions;
    std::vector<float> queue_priorities;
    std::vector<VkDeviceQueueCreateInfo> queue_infos;
    VkDeviceCreateInfo device_info;
};
