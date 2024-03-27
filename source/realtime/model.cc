//
// MIT License
//
// Copyright (c) 2023 Elias Engelbert Plank
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

#ifdef _MSC_VER
#pragma warning(disable : 4201)
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <array>
#include <cstring>
#include <unordered_map>
#include "model.h"
#include "tinyobjloader.h"

template<>
struct std::hash<rt::Model::Vertex> {
    usize operator()(rt::Model::Vertex const &vertex) const noexcept {
        usize seed = 0;
        rt::hash_combine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
        return seed;
    }
};

namespace rt {

/// Retrieves the binding descriptions for a vertex
std::vector<VkVertexInputBindingDescription> Model::Vertex::binding_descriptions() {
    return { { 0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX } };
}

/// Retrieves the attribute descriptions for a vertex
std::vector<VkVertexInputAttributeDescription> Model::Vertex::attribute_descriptions() {
    std::vector<VkVertexInputAttributeDescription> attributes;
    auto &position = attributes.emplace_back();
    position.binding = 0;
    position.location = 0;
    position.format = VK_FORMAT_R32G32B32_SFLOAT;
    position.offset = offsetof(Vertex, position);

    auto &color = attributes.emplace_back();
    color.binding = 0;
    color.location = 1;
    color.format = VK_FORMAT_R32G32B32_SFLOAT;
    color.offset = offsetof(Vertex, color);
    return attributes;
}

/// Loads a model from the specified filesystem path
void Model::Builder::load_model(const fs::path &path) {
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

                auto color_index = 3 * index.vertex_index + 2;
                if (color_index < attrib.colors.size()) {
                    vertex.color = {
                        attrib.colors[color_index - 2],
                        attrib.colors[color_index - 1],
                        attrib.colors[color_index - 0],
                    };
                } else {
                    vertex.color = { 1.0f, 1.0f, 1.0f };
                }
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

/// Creates a new model
Model::Model(Device &device, const Builder &builder)
    : device{ device },
      vertex_buffer{},
      vertex_buffer_memory{},
      vertex_count{},
      has_index_buffer{ false },
      index_buffer{},
      index_buffer_memory{},
      index_count{} {
    create_vertex_buffers(builder.vertices);
    create_index_buffers(builder.indices);
}

/// Destroys the data of the current model
Model::~Model() {
    vkDestroyBuffer(device.logical_device, vertex_buffer, nullptr);
    vkFreeMemory(device.logical_device, vertex_buffer_memory, nullptr);
    if (has_index_buffer) {
        vkDestroyBuffer(device.logical_device, index_buffer, nullptr);
        vkFreeMemory(device.logical_device, index_buffer_memory, nullptr);
    }
}

/// Creates a model from the specified filesystem path
std::unique_ptr<Model> Model::create_from_file(Device &device, const fs::path &path) {
    Builder builder{};
    builder.load_model(path);
    return std::make_unique<Model>(device, builder);
}

/// Binds the current model using the specified command buffer
void Model::bind(VkCommandBuffer command_buffer) const {
    std::array<VkDeviceSize, 1> offsets = { 0 };
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer, offsets.data());
    if (has_index_buffer) {
        vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);
    }
}

/// Draws the model using the specified command buffer
void Model::draw(VkCommandBuffer command_buffer) const {
    if (has_index_buffer) {
        vkCmdDrawIndexed(command_buffer, index_count, 1, 0, 0, 0);
    } else {
        vkCmdDraw(command_buffer, vertex_count, 1, 0, 0);
    }
}

void Model::create_vertex_buffers(const std::vector<Vertex> &vertices) {
    vertex_count = static_cast<u32>(vertices.size());
    assert(vertex_count >= 3 and "[model] Vertex count must be at least 3!");

    auto buffer_size = sizeof(Vertex) * vertex_count;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    device.create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer,
                         staging_buffer_memory);

    void *data = nullptr;
    vkMapMemory(device.logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);
    std::memcpy(data, vertices.data(), buffer_size);
    vkUnmapMemory(device.logical_device, staging_buffer_memory);

    device.create_buffer(buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex_buffer, vertex_buffer_memory);
    device.copy_buffer(staging_buffer, vertex_buffer, buffer_size);

    vkDestroyBuffer(device.logical_device, staging_buffer, nullptr);
    vkFreeMemory(device.logical_device, staging_buffer_memory, nullptr);
}

/// Creates the index buffers for the current model
void Model::create_index_buffers(const std::vector<u32> &indices) {
    index_count = static_cast<u32>(indices.size());
    has_index_buffer = index_count > 0;
    if (not has_index_buffer) {
        return;
    }

    auto buffer_size = sizeof(u32) * index_count;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    device.create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer,
                         staging_buffer_memory);

    void *data = nullptr;
    vkMapMemory(device.logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);
    std::memcpy(data, indices.data(), buffer_size);
    vkUnmapMemory(device.logical_device, staging_buffer_memory);

    device.create_buffer(buffer_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, index_buffer, index_buffer_memory);
    device.copy_buffer(staging_buffer, index_buffer, buffer_size);

    vkDestroyBuffer(device.logical_device, staging_buffer, nullptr);
    vkFreeMemory(device.logical_device, staging_buffer_memory, nullptr);
}

}// namespace rt
