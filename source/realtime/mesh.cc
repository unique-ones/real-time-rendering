//
// MIT License
//
// Copyright (c) 2024 Elias Engelbert Plank
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "vulkan/vulkan_core.h"
#ifdef _MSC_VER
#pragma warning(disable : 4201)
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <array>
#include <cstring>
#include <unordered_map>
#include "mesh.h"
#include "tinyobjloader.h"

template<>
struct std::hash<rt::Mesh::Vertex> {
    usize operator()(rt::Mesh::Vertex const &vertex) const noexcept {
        usize seed = 0;
        rt::hash_combine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
        return seed;
    }
};

namespace rt {

/// Retrieves the binding descriptions for a vertex
std::vector<VkVertexInputBindingDescription> Mesh::Vertex::binding_descriptions() {
    return { { 0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX } };
}

/// Retrieves the attribute descriptions for a vertex
std::vector<VkVertexInputAttributeDescription> Mesh::Vertex::attribute_descriptions() {
    std::vector<VkVertexInputAttributeDescription> attributes;
    attributes.emplace_back(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position));
    attributes.emplace_back(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color));
    attributes.emplace_back(2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal));
    attributes.emplace_back(3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv));
    return attributes;
}

/// Loads a wavefront mesh from the specified filesystem path
void Mesh::Builder::from_wavefront(const fs::path &path) {
    tinyobj::attrib_t attrib{};
    std::vector<tinyobj::shape_t> shapes{};
    std::vector<tinyobj::material_t> materials{};
    std::string warning;
    std::string err;

    if (not tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &err, path.string().c_str())) {
        error(64, warning + err);
    }

    vertices.clear();
    indices.clear();

    std::unordered_map<Vertex, u32> unique_vertices{};

    for (const auto &shape : shapes) {
        for (const auto &index : shape.mesh.indices) {
            Vertex vertex{};
            if (index.vertex_index >= 0) {
                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2],
                };
                vertex.color = {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2],
                };
            }
            if (index.normal_index >= 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2],
                };
            }
            if (index.texcoord_index >= 0) {
                vertex.uv = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1],
                };
            }

            if (not unique_vertices.contains(vertex)) {
                unique_vertices[vertex] = static_cast<u32>(vertices.size());
                vertices.push_back(vertex);
            }
            indices.push_back(unique_vertices[vertex]);
        }
    }
}

/// Creates a new mesh
Mesh::Mesh(Device &device, const Builder &builder)
    : centroid{},
      device{ device },
      vertex_buffer{},
      vertex_count{},
      has_index_buffer{ false },
      index_buffer{},
      index_count{} {
    create_vertex_buffers(builder.vertices);
    create_index_buffers(builder.indices);
    compute_centroid(builder.vertices);
}

/// Destroys the data of the current mesh
Mesh::~Mesh() = default;

/// Creates a mesh from the specified filesystem path
std::unique_ptr<Mesh> Mesh::from_wavefront(Device &device, const fs::path &path) {
    Builder builder{};
    builder.from_wavefront(path);
    return std::make_unique<Mesh>(device, builder);
}

/// Binds the current mesh using the specified command buffer
void Mesh::bind(VkCommandBuffer command_buffer) const {
    std::array<VkDeviceSize, 1> offsets = { 0 };
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer->buffer, offsets.data());
    if (has_index_buffer) {
        vkCmdBindIndexBuffer(command_buffer, index_buffer->buffer, 0, VK_INDEX_TYPE_UINT32);
    }
}

/// Draws the mesh using the specified command buffer
void Mesh::draw(VkCommandBuffer command_buffer) const {
    if (has_index_buffer) {
        vkCmdDrawIndexed(command_buffer, index_count, 1, 0, 0, 0);
    } else {
        vkCmdDraw(command_buffer, vertex_count, 1, 0, 0);
    }
}

void Mesh::create_vertex_buffers(const std::vector<Vertex> &vertices) {
    vertex_count = static_cast<u32>(vertices.size());
    assert(vertex_count >= 3 and "[mesh] Vertex count must be at least 3!");

    auto buffer_size = sizeof(Vertex) * vertex_count;
    auto vertex_size = sizeof(Vertex);

    Buffer staging_buffer{ device, vertex_size, vertex_count, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

    staging_buffer.map();
    staging_buffer.write((void *) vertices.data());

    vertex_buffer = std::make_unique<Buffer>(device, vertex_size, vertex_count,
                                             VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    device.copy_buffer(staging_buffer.buffer, vertex_buffer->buffer, buffer_size);
}

/// Creates the index buffers for the current mesh
void Mesh::create_index_buffers(const std::vector<u32> &indices) {
    index_count = static_cast<u32>(indices.size());
    has_index_buffer = index_count > 0;
    if (not has_index_buffer) {
        return;
    }

    auto buffer_size = sizeof(u32) * index_count;
    auto index_size = sizeof(u32);

    Buffer staging_buffer{ device, index_size, index_count, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

    staging_buffer.map();
    staging_buffer.write((void *) indices.data());

    index_buffer = std::make_unique<Buffer>(device, index_size, index_count,
                                            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    device.copy_buffer(staging_buffer.buffer, index_buffer->buffer, buffer_size);
}

/// Computes the centroid of the current mesh
void Mesh::compute_centroid(const std::vector<Vertex> &vertices) {
    auto result = glm::vec3{ 0.0f };
    for (const auto &vertex : vertices) {
        result += vertex.position;
    }
    centroid = result / static_cast<f32>(vertices.size());
}

}// namespace rt
