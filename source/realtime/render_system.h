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

#ifndef REALTIME_RENDER_SYSTEM_H
#define REALTIME_RENDER_SYSTEM_H

#include <memory>
#include <vector>

#include "camera.h"
#include "device.h"
#include "entity.h"
#include "pipeline.h"

namespace rt {

class RenderSystem {
public:
    /// Creates a realtime render system
    /// @param device The device
    /// @param render_pass The render pass
    explicit RenderSystem(Device &device, VkRenderPass render_pass);

    /// Destroys all render system objects
    ~RenderSystem();

    /// A render system cannot be copied or moved
    RenderSystem(const RenderSystem &) = delete;
    RenderSystem &operator=(const RenderSystem &) = delete;
    RenderSystem(RenderSystem &&) = delete;
    RenderSystem &operator=(RenderSystem &&) = delete;

    /// Renders the entities
    /// @param command_buffer The command buffer
    /// @param entities The entities to render
    /// @param camera The camera
    /// @param dt The delta time
    void render_entities(VkCommandBuffer command_buffer, std::vector<Entity> &entities, const Camera &camera, f32 dt);

private:
    /// Creates the layout of the pipeline
    void create_pipeline_layout();

    /// Creates the pipeline
    void create_pipeline(VkRenderPass render_pass);

    Device &device;
    std::unique_ptr<Pipeline> pipeline;
    VkPipelineLayout pipeline_layout;
};

}// namespace rt

#endif// REALTIME_RENDER_SYSTEM_H
