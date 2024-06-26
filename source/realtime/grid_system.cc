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

#include "grid_system.h"
#include "render_system.h"

namespace rt {

/// Creates a realtime grid system
GridSystem::GridSystem(Device &device, VkRenderPass render_pass) : device{ device }, pipeline_layout{ } {
    create_pipeline_layout();
    create_pipeline(render_pass);
    create_grid();
}

/// Destroys all grid system objects
GridSystem::~GridSystem() {
    vkDestroyPipelineLayout(device.logical_device, pipeline_layout, nullptr);
}

/// Renders the grid
void GridSystem::render([[maybe_unused]] const FrameInfo &info, [[maybe_unused]] f32 size) {
}

/// Creates the layout of the pipeline
void GridSystem::create_pipeline_layout() {
    VkPushConstantRange range{};
    range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    range.offset = 0;
    range.size = sizeof(PushConstantData);

    VkPipelineLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_info.setLayoutCount = 0;
    layout_info.pSetLayouts = nullptr;
    layout_info.pushConstantRangeCount = 1;
    layout_info.pPushConstantRanges = &range;

    if (vkCreatePipelineLayout(device.logical_device, &layout_info, nullptr, &pipeline_layout) != VK_SUCCESS) {
        error(64, "[grid system] Unable to create pipeline layout!");
    }
}

/// Creates the pipeline
void GridSystem::create_pipeline(VkRenderPass render_pass) {
    assert(pipeline_layout and "[grid system] Cannot create pipeline before pipeline layout!");

    PipelineDescription desc{};
    PipelineDescription::default_description(desc);
    desc.render_pass = render_pass;
    desc.pipeline_layout = pipeline_layout;
    pipeline = std::make_unique<Pipeline>(device, "shaders/grid.vert.spv", "shaders/grid.frag.spv", desc);
}

/// Creates the grid
void GridSystem::create_grid() {
    // Mesh::Builder builder{};
    // grid = std::make_unique<Mesh>(device, builder);
}

}// namespace rt