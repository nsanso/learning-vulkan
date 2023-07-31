#pragma once

#include <vk_mem_alloc.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <glm/gtx/transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

struct AllocatedBuffer {
    VkBuffer buffer;
    VmaAllocation allocation;
};

inline void vk_check(VkResult err) {
#ifndef NDEBUG
    if (err) {
        printf("Detected Vulkan error: %d\n", err);
        assert(!err);
    }
#endif
}
