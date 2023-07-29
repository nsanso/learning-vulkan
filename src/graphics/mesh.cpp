#include "mesh.h"

#include <cstring>

#include "utils.h"

Mesh::Mesh(VmaAllocator allocator, std::vector<Vertex> vertices)
    : vertices(vertices), allocator(allocator) {
    VkBufferCreateInfo buffer_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = vertices.size() * sizeof(Vertex),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    };

    VmaAllocationCreateInfo allocation_info{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };

    vk_check(vmaCreateBuffer(allocator, &buffer_info, &allocation_info,
                             &vertex_buffer.buffer, &vertex_buffer.allocation,
                             nullptr));
    void *data;
    vmaMapMemory(allocator, vertex_buffer.allocation, &data);
    memcpy(data, vertices.data(), vertices.size() * sizeof(Vertex));
    vmaUnmapMemory(allocator, vertex_buffer.allocation);
}

void Mesh::destroy() {
    vmaDestroyBuffer(allocator, vertex_buffer.buffer, vertex_buffer.allocation);
}
