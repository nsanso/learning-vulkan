#pragma once

#include <glm/vec3.hpp>
#include <vector>

#include "utils.h"

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;

    const static std::vector<VkVertexInputBindingDescription> get_bindings() {
        return std::vector<VkVertexInputBindingDescription>{
            VkVertexInputBindingDescription{
                .binding = 0,
                .stride = sizeof(Vertex),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            }};
    };

    const static std::vector<VkVertexInputAttributeDescription>
    get_attributes() {
        return std::vector<VkVertexInputAttributeDescription>{
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
        };
    };

    const static VkPipelineVertexInputStateCreateFlags get_flags() {
        return 0;
    };
};

class Mesh {
   public:
    Mesh(VmaAllocator allocator, std::vector<Vertex> vertices);
    std::vector<Vertex> vertices;
    AllocatedBuffer vertex_buffer;
    void destroy();

   private:
    VmaAllocator allocator;
};
