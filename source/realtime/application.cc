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
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "application.h"
#include "utility.h"

namespace rt {

    /// Creates a realtime application
    Application::Application(Specification specification)
        : window(std::move(specification)),
          device(window),
          swapchain(device, window.extent()),
          pipeline_layout{} {
        create_pipeline_layout();
        create_pipeline();
        create_command_buffers();
    }

    Application::~Application() {
        vkDestroyPipelineLayout(device.logical_device, pipeline_layout, nullptr);
    }


    /// Runs the application
    void Application::run() {
        while (!window.should_close()) {
            glfwPollEvents();
        }
    }

    /// Creates the layout of the pipeline
    void Application::create_pipeline_layout() {
        VkPipelineLayoutCreateInfo layout_info{};
        layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layout_info.setLayoutCount = 0;
        layout_info.pSetLayouts = nullptr;
        layout_info.pushConstantRangeCount = 0;
        layout_info.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(device.logical_device, &layout_info, nullptr, &pipeline_layout) != VK_SUCCESS) {
            error(64, "[application] Unable to create pipeline layout!");
        }
    }

    /// Creates the pipeline
    void Application::create_pipeline() {
        auto desc = PipelineDescription::default_description(swapchain.width(), swapchain.height());
        desc.render_pass = swapchain.render_pass;
        desc.pipeline_layout = pipeline_layout;
        pipeline = std::make_unique<Pipeline>(device, "shaders/simple.vert.spv", "shaders/simple.frag.spv", desc);
    }

    /// Creates the command buffers
    void Application::create_command_buffers() { }

    /// Draws a frame
    void Application::draw_frame() { }

}// namespace rt
