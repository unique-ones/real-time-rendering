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

#ifndef REALTIME_GRID_SYSTEM_H
#define REALTIME_GRID_SYSTEM_H

#include <memory>
#include "device.h"
#include "pipeline.h"
#include "frame_info.h"

namespace rt {

class GridSystem {
public:
    /// Creates a realtime grid system
    /// @param device The device
    /// @param render_pass The render pass
    explicit GridSystem(Device &device, VkRenderPass render_pass);

    /// Destroys all grid system objects
    ~GridSystem();

    /// A grid system cannot be copied or moved
    GridSystem(const GridSystem &) = delete;
    GridSystem &operator=(const GridSystem &) = delete;
    GridSystem(GridSystem &&) = delete;
    GridSystem &operator=(GridSystem &&) = delete;

    /// Renders the grid
    /// @param info The frame info
    /// @param size The grid size
    void render(const FrameInfo &info, f32 size);

private:
    /// Creates the layout of the pipeline
    void create_pipeline_layout();

    /// Creates the pipeline
    void create_pipeline(VkRenderPass render_pass);

    /// Creates the grid
    void create_grid();

    Device &device;
    std::unique_ptr<Pipeline> pipeline;
    VkPipelineLayout pipeline_layout;
    std::unique_ptr<Mesh> grid;
};

}// namespace rt

#endif// REALTIME_GRID_SYSTEM_H
