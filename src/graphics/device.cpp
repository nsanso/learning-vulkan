#include "device.h"

#include <cstdio>

#include "utils.h"

VkQueue GraphicsDevice::get_queue(uint32_t family) {
    for (GraphicsQueue queue : queues) {
        if (queue.family == family) {
            return queue.handle;
        }
    }
    printf("queue not found (family %u\n", family);
    // I'm really hoping that 0 is not a valid queue handle, otherwise I'll have
    // to use std::optional
    return 0;
}

void GraphicsDevice::destroy() {
    queues.clear();
    vkDestroyDevice(device, nullptr);
    device = {};
}

GraphicsDeviceBuilder *GraphicsDeviceBuilder::add_device_extension(
    const char *extension) {
    device_extensions.push_back(extension);
    return this;
};

GraphicsDeviceBuilder *GraphicsDeviceBuilder::add_queue(uint32_t family,
                                                        float priority) {
    queue_priorities.push_back(priority);

    VkDeviceQueueCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = (uint32_t)family,
        .queueCount = 1,
        .pQueuePriorities = &queue_priorities.at(queue_priorities.size() - 1),
    };

    queue_infos.push_back(info);
    return this;
};

GraphicsDevice GraphicsDeviceBuilder::build() {
    GraphicsDevice destination{};

    device_info.queueCreateInfoCount = (uint32_t)queue_infos.size();
    device_info.pQueueCreateInfos = queue_infos.data();
    device_info.enabledExtensionCount = (uint32_t)device_extensions.size();
    device_info.ppEnabledExtensionNames = device_extensions.data();

    vk_check(vkCreateDevice(physical_device, &device_info, nullptr,
                            &destination.device));

    destination.queues.resize(queue_infos.size());
    for (size_t i = 0; i < queue_infos.size(); i++) {
        GraphicsQueue queue{
            .family = queue_infos.at(i).queueFamilyIndex,
        };
        vkGetDeviceQueue(destination.device, queue.family, i, &queue.handle);
        destination.queues.at(i) = queue;
    }

    return destination;
}
