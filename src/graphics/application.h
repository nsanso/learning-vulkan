#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

class GraphicsApplication {
   public:
    VkInstance instance;
    VkPhysicalDevice device;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    std::vector<VkQueueFamilyProperties> queue_families;

    size_t get_queue_family(VkQueueFlags flags);
    void destroy();
};

class GraphicsApplicationBuilder {
   public:
    GraphicsApplication build();
    GraphicsApplicationBuilder *set_application_name(std::string name);
    GraphicsApplicationBuilder *set_application_version(uint32_t major,
                                                        uint32_t minor,
                                                        uint32_t patch);
    GraphicsApplicationBuilder *add_validation_layer(const char *layer);
    GraphicsApplicationBuilder *add_instance_extension(const char *extension);
    GraphicsApplicationBuilder *add_instance_extension(
        std::vector<const char *> extensions);
    GraphicsApplicationBuilder *add_device_extension(const char *layer);

    GraphicsApplicationBuilder()
        : application_info(VkApplicationInfo{
              .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
              .pApplicationName = "Application",
              .applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
              .pEngineName = "Custom",
              .engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
              .apiVersion = VK_API_VERSION_1_3,
          }),

          instance_extensions(std::vector<const char *>{
#ifndef NDEBUG
              VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
          }),

          validation_layers(std::vector<const char *>{
#ifndef NDEBUG
              "VK_LAYER_KHRONOS_validation",
#endif
          }),

          instance_info(VkInstanceCreateInfo{
              .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
              .pApplicationInfo = &application_info,
          }) {
    }

   private:
    VkApplicationInfo application_info;

    std::vector<const char *> instance_extensions;
    std::vector<const char *> validation_layers;
    VkInstanceCreateInfo instance_info;
};
