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

#ifndef REALTIME_MODEL_H
#define REALTIME_MODEL_H

/// Force angles to be specified in radians
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "device.h"

namespace rt {

class Model {
public:
    struct Vertex {
        glm::vec2 position;
        glm::vec3 color;

        /// Retrieves the binding descriptions for a vertex
        /// @return The binding decriptions for a vertex
        static std::vector<VkVertexInputBindingDescription> binding_descriptions();

        /// Retrieves the attribute descriptions for a vertex
        /// @return The attribute descriptions for a vertex
        static std::vector<VkVertexInputAttributeDescription> attribute_descriptions();
    };

    /// Creates a new model
    /// @param device The device instance
    /// @param vertices The vertices
    explicit Model(Device &device, const std::vector<Vertex> &vertices);

    /// Destroys the data of the current model
    ~Model();

    /// A model cannot be copied
    Model(const Model &) = delete;
    Model &operator=(const Model &) = delete;

    /// Binds the current model using the specified command buffer
    /// @param command_buffer The recording command buffer
    void bind(VkCommandBuffer command_buffer) const;

    /// Draws the model using the specified command buffer
    /// @param command_buffer The recording command buffer
    void draw(VkCommandBuffer command_buffer) const;

private:
    /// Creates the vertex buffers for the current model
    /// @param vertices The vertices
    void create_vertex_buffers(const std::vector<Vertex> &vertices);

    Device &device;
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;
    u32 vertex_count;
};

}// namespace rt

#endif// REALTIME_MODEL_H
