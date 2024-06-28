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

#ifndef REALTIME_mesh_H
#define REALTIME_mesh_H

/// Force angles to be specified in radians
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>

#include "buffer.h"
#include "device.h"
#include "utility.h"


namespace rt {

class Mesh {
public:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec3 normal{};
        glm::vec2 uv{};

        /// Retrieves the binding descriptions for a vertex
        /// @return The binding decriptions for a vertex
        static std::vector<VkVertexInputBindingDescription> binding_descriptions();

        /// Retrieves the attribute descriptions for a vertex
        /// @return The attribute descriptions for a vertex
        static std::vector<VkVertexInputAttributeDescription> attribute_descriptions();

        /// Checks whether to mesh vertices are equal
        /// @param other The other vertex
        /// @return Partial ordering
        auto operator<=>(const Vertex &other) const = default;
    };

    struct Builder {
        std::vector<Vertex> vertices{};
        std::vector<u32> indices{};

        /// Loads a wavefront mesh from the specified filesystem path
        /// @param path The filesystem path of the mesh
        void from_wavefront(const fs::path &path);
    };


    /// The centroid of the mesh
    glm::vec3 centroid;

    /// Creates a new mesh
    /// @param device The device instance
    /// @param builder A builder for the vertex data
    explicit Mesh(Device &device, const Builder &builder);

    /// Destroys the data of the current mesh
    ~Mesh();

    /// A mesh cannot be copied
    Mesh(const Mesh &) = delete;
    Mesh &operator=(const Mesh &) = delete;

    /// Creates a mesh from the specified filesystem path
    /// @param device The device instance
    /// @param path The filesystem path of the mesh
    /// @return A new mesh
    static std::unique_ptr<Mesh> from_wavefront(Device &device, const fs::path &path);

    /// Binds the current mesh using the specified command buffer
    /// @param command_buffer The recording command buffer
    void bind(VkCommandBuffer command_buffer) const;

    /// Draws the mesh using the specified command buffer
    /// @param command_buffer The recording command buffer
    void draw(VkCommandBuffer command_buffer) const;

private:
    /// Creates the vertex buffers for the current mesh
    /// @param vertices The vertices
    void create_vertex_buffers(const std::vector<Vertex> &vertices);

    /// Creates the index buffers for the current mesh
    /// @param indices The indices
    void create_index_buffers(const std::vector<u32> &indices);

    /// Computes the centroid of the current mesh
    /// @param vertices The vertices
    void compute_centroid(const std::vector<Vertex> &vertices);

    Device &device;

    std::unique_ptr<Buffer> vertex_buffer;
    u32 vertex_count;

    bool has_index_buffer;
    std::unique_ptr<Buffer> index_buffer;
    u32 index_count;
};

}// namespace rt

#endif// REALTIME_mesh_H
