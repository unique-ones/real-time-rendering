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

#include <array>

#include "model.h"

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
    position.format = VK_FORMAT_R32G32_SFLOAT;
    position.offset = offsetof(Vertex, position);

    auto &color = attributes.emplace_back();
    color.binding = 0;
    color.location = 1;
    color.format = VK_FORMAT_R32G32B32_SFLOAT;
    color.offset = offsetof(Vertex, color);
    return attributes;
}

/// Creates a new model
Model::Model(Device &device, const std::vector<Vertex> &vertices)
    : device{ device },
      vertex_buffer{},
      vertex_buffer_memory{},
      vertex_count{} {
    create_vertex_buffers(vertices);
}

/// Destroys the data of the current model
Model::~Model() {
    vkDestroyBuffer(device.logical_device, vertex_buffer, nullptr);
    vkFreeMemory(device.logical_device, vertex_buffer_memory, nullptr);
}

/// Binds the current model using the specified command buffer
void Model::bind(VkCommandBuffer command_buffer) const {
    std::array<VkDeviceSize, 1> offsets = { 0 };
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer, offsets.data());
}

/// Draws the model using the specified command buffer
void Model::draw(VkCommandBuffer command_buffer) const {
    vkCmdDraw(command_buffer, vertex_count, 1, 0, 0);
}

void Model::create_vertex_buffers(const std::vector<Vertex> &vertices) {
    vertex_count = static_cast<u32>(vertices.size());
    assert(vertex_count >= 3 && "[model] Vertex count must be at least 3!");

    auto buffer_size = sizeof(Vertex) * vertex_count;
    device.create_buffer(buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertex_buffer,
                         vertex_buffer_memory);

    void *data = nullptr;
    vkMapMemory(device.logical_device, vertex_buffer_memory, 0, buffer_size, 0, &data);
    std::memcpy(data, vertices.data(), buffer_size);
    vkUnmapMemory(device.logical_device, vertex_buffer_memory);
}

}// namespace rt
