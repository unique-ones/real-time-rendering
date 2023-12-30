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

#include <array>

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
            draw_frame();
        }

        vkDeviceWaitIdle(device.logical_device);
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
    void Application::create_command_buffers() {
        command_buffers.resize(swapchain.image_count());

        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = device.command_pool;
        alloc_info.commandBufferCount = static_cast<u32>(command_buffers.size());

        if (vkAllocateCommandBuffers(device.logical_device, &alloc_info, command_buffers.data()) != VK_SUCCESS) {
            error(64, "[application] Failed to allocate command buffers!");
        }

        for (s32 i = 0; i < command_buffers.size(); ++i) {
            VkCommandBufferBeginInfo begin_info{};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            if (vkBeginCommandBuffer(command_buffers[i], &begin_info) != VK_SUCCESS) {
                error(64, "[application] Failed to begin recording command buffer!");
            }

            VkRenderPassBeginInfo render_pass_info{};
            render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            render_pass_info.renderPass = swapchain.render_pass;
            render_pass_info.framebuffer = swapchain.framebuffer_at(i);
            render_pass_info.renderArea.offset = { 0, 0 };
            render_pass_info.renderArea.extent = swapchain.swapchain_extent;

            std::array<VkClearValue, 2> clear_values{};
            clear_values[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
            clear_values[1].depthStencil = { 1.0f, 0 };
            render_pass_info.clearValueCount = static_cast<u32>(clear_values.size());
            render_pass_info.pClearValues = clear_values.data();

            vkCmdBeginRenderPass(command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

            pipeline->bind(command_buffers[i]);
            vkCmdDraw(command_buffers[i], 3, 1, 0, 0);

            vkCmdEndRenderPass(command_buffers[i]);
            if (vkEndCommandBuffer(command_buffers[i]) != VK_SUCCESS) {
                error(64, "[application] Failed to record command buffer!");
            }
        }
    }

    /// Draws a frame
    void Application::draw_frame() {
        u32 image_index;
        if (auto result = swapchain.acquire_next_image(&image_index);
            result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            error(64, "[application] Failed to acquire swap chain image!");
        }

        if (swapchain.submit_command_buffers(&command_buffers[image_index], &image_index) != VK_SUCCESS) {
            error(64, "[application] Failed to submit command buffers for drawing!");
        }
    }

}// namespace rt
