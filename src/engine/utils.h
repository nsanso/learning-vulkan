#pragma once

#include <vk_mem_alloc.h>

#include <cstdio>
#include <cstdlib>

struct AllocatedBuffer {
    VkBuffer buffer;
    VmaAllocation allocation;
};

inline void vk_check(VkResult err) {
    if (err) {
        printf("Detected Vulkan error: %d\n", err);
        abort();
    }
}
