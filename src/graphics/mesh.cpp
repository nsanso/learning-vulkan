#include "mesh.h"

#include "utils.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <cstring>
#include <string>

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

    assert(!vmaCreateBuffer(allocator, &buffer_info, &allocation_info,
                             &vertex_buffer.buffer, &vertex_buffer.allocation,
                             nullptr));
    void* data;
    vmaMapMemory(allocator, vertex_buffer.allocation, &data);
    memcpy(data, vertices.data(), vertices.size() * sizeof(Vertex));
    vmaUnmapMemory(allocator, vertex_buffer.allocation);
}

void Mesh::destroy() {
    vmaDestroyBuffer(allocator, vertex_buffer.buffer, vertex_buffer.allocation);
}

std::optional<Mesh> Mesh::from_obj(VmaAllocator allocator,
                                   const char* directory,
                                   const char* filename) {
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = directory;
    reader_config.triangulate = true;

    tinyobj::ObjReader reader;

    const std::string inputfile = std::string(directory) + filename;

    reader.ParseFromFile(inputfile, reader_config);

    if (!reader.Error().empty()) {
        printf("TinyObjReader: %s\n", reader.Error().c_str());
        return std::optional<Mesh>{};
    }

    if (!reader.Warning().empty()) {
        printf("TinyObjReader: %s\n", reader.Warning().c_str());
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    std::vector<Vertex> vertices{};

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                Vertex vertex{};

                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx =
                    attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy =
                    attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz =
                    attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                vertex.position = glm::vec3{vx, vy, vz};

                // Check if `normal_index` is zero or positive. negative = no
                // normal data
                if (idx.normal_index >= 0) {
                    tinyobj::real_t nx =
                        attrib.normals[3 * size_t(idx.normal_index) + 0];
                    tinyobj::real_t ny =
                        attrib.normals[3 * size_t(idx.normal_index) + 1];
                    tinyobj::real_t nz =
                        attrib.normals[3 * size_t(idx.normal_index) + 2];

                    vertex.normal = glm::vec3{nx, ny, nz};
                }

                // Check if `texcoord_index` is zero or positive. negative = no
                // texcoord data
                if (idx.texcoord_index >= 0) {
                    tinyobj::real_t tx =
                        attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    tinyobj::real_t ty =
                        attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                }

                // Optional: vertex colors
                tinyobj::real_t red =
                    attrib.colors[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t green =
                    attrib.colors[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t blue =
                    attrib.colors[3 * size_t(idx.vertex_index) + 2];

                vertex.color = glm::vec3{red, green, blue};

                vertices.push_back(vertex);
            }
            index_offset += fv;
        }
    }

    return std::optional<Mesh>(Mesh(allocator, vertices));
}
