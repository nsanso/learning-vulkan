#pragma once

#include <vk_mem_alloc.h>

#include <glm/vec3.hpp>
#include <vector>

struct AllocatedBuffer {
    VkBuffer buffer;
    VmaAllocation allocation;
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
};

struct Mesh {
    AllocatedBuffer vertex_buffer;
    std::vector<Vertex> vertices;
};

const struct {
    VkVertexInputBindingDescription bindings[1];
    VkVertexInputAttributeDescription attributes[3];
    VkPipelineVertexInputStateCreateFlags flags;
} VertexInputDescription{.bindings{
                             VkVertexInputBindingDescription{
                                 .binding = 0,
                                 .stride = sizeof(Vertex),
                                 .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
                             },
                         },
                         .attributes{
                             VkVertexInputAttributeDescription{
                                 .location = 0,
                                 .binding = 0,
                                 .format = VK_FORMAT_R32G32B32_SFLOAT,
                                 .offset = offsetof(Vertex, position),
                             },

                             VkVertexInputAttributeDescription{
                                 .location = 1,
                                 .binding = 0,
                                 .format = VK_FORMAT_R32G32B32_SFLOAT,
                                 .offset = offsetof(Vertex, normal),
                             },

                             VkVertexInputAttributeDescription{
                                 .location = 2,
                                 .binding = 0,
                                 .format = VK_FORMAT_R32G32B32_SFLOAT,
                                 .offset = offsetof(Vertex, color),
                             },
                         },
                         .flags = 0};
