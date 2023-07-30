#include "application.h"

#include <cstdio>

#include "utils.h"

size_t GraphicsApplication::get_queue_family(VkQueueFlags flags) {
    for (size_t i = 0; i < queue_families.size(); i++) {
        if (queue_families.at(i).queueFlags & flags) {
            return i;
        }
    }
    return -1;
}

void GraphicsApplication::destroy() {
    device = {};
    properties = {};
    features = {};
    queue_families.clear();

    vkDestroyInstance(instance, nullptr);
    instance = {};
}

GraphicsApplicationBuilder *GraphicsApplicationBuilder::set_application_name(
    std::string name) {
    application_info.pApplicationName = name.c_str();
    return this;
};

GraphicsApplicationBuilder *GraphicsApplicationBuilder::set_application_version(
    uint32_t major, uint32_t minor, uint32_t patch) {
    application_info.applicationVersion =
        VK_MAKE_API_VERSION(0, major, minor, patch);
    return this;
};

GraphicsApplicationBuilder *GraphicsApplicationBuilder::add_validation_layer(
    const char *layer) {
    validation_layers.push_back(layer);
    return this;
};

GraphicsApplicationBuilder *GraphicsApplicationBuilder::add_instance_extension(
    const char *extension) {
    instance_extensions.push_back(extension);
    return this;
};

GraphicsApplicationBuilder *GraphicsApplicationBuilder::add_instance_extension(
    std::vector<const char *> extensions) {
    instance_extensions.insert(instance_extensions.end(), extensions.begin(),
                               extensions.end());
    return this;
};

GraphicsApplication GraphicsApplicationBuilder::build() {
    GraphicsApplication destination{};

    // TODO: setup debugging messages
    // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Validation_layers#page_Message-callback

    instance_info.enabledLayerCount = (uint32_t)validation_layers.size();
    instance_info.ppEnabledLayerNames = validation_layers.data();
    instance_info.enabledExtensionCount = (uint32_t)instance_extensions.size();
    instance_info.ppEnabledExtensionNames = instance_extensions.data();

    vk_check(vkCreateInstance(&instance_info, nullptr, &destination.instance));

    // Get device
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(destination.instance, &device_count, nullptr);

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(destination.instance, &device_count,
                               devices.data());

    for (auto device : devices) {
        vkGetPhysicalDeviceProperties(device, &destination.properties);
        vkGetPhysicalDeviceFeatures(device, &destination.features);

        uint32_t qfam_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &qfam_count, nullptr);

        destination.queue_families.resize(qfam_count);
        vkGetPhysicalDeviceQueueFamilyProperties(
            device, &qfam_count, destination.queue_families.data());

        // TODO: add checks that select a suitable gpu instead of just taking
        // the first one
        if (true) {
            destination.device = device;
            break;
        }
    }

    return destination;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
              void *pUserData) {
    printf("Validation layer: %s\n", pCallbackData->pMessage);
    return VK_FALSE;
}
