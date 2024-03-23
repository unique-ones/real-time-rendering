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

/// Force angles to be specified in radians
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "application.h"
#include "utility.h"

namespace rt {

struct PushConstantData {
    glm::vec2 offset;
    alignas(16) glm::vec3 color;
};

/// Creates a realtime application
Application::Application(Specification specification)
    : window(std::move(specification)),
      device(window),
      pipeline_layout{} {
    load_models();
    create_pipeline_layout();
    recreate_swapchain();
    create_command_buffers();
}

/// Destroys all application objects
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

/// Loads the models
void Application::load_models() {
    // clang-format off
    std::vector vertices = {
        Model::Vertex{ { -0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f } },
        Model::Vertex{ { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f } },
        Model::Vertex{ { 0.0f, -0.5f }, { 0.0f, 0.0f, 1.0f } }
    };
    // clang-format on

    model = std::make_unique<Model>(device, vertices);
}

/// Creates the layout of the pipeline
void Application::create_pipeline_layout() {
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
        error(64, "[application] Unable to create pipeline layout!");
    }
}

/// Creates the pipeline
void Application::create_pipeline() {
    assert(swapchain && "[application] Cannot create pipeline before swapchain!");
    assert(pipeline_layout && "[application] Cannot create pipeline before pipeline layout!");

    PipelineDescription desc{};
    PipelineDescription::default_description(desc);
    desc.render_pass = swapchain->render_pass;
    desc.pipeline_layout = pipeline_layout;
    pipeline = std::make_unique<Pipeline>(device, "shaders/simple.vert.spv", "shaders/simple.frag.spv", desc);
}

/// Creates the command buffers
void Application::create_command_buffers() {
    command_buffers.resize(swapchain->image_count());

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = device.command_pool;
    alloc_info.commandBufferCount = static_cast<u32>(command_buffers.size());

    if (vkAllocateCommandBuffers(device.logical_device, &alloc_info, command_buffers.data()) != VK_SUCCESS) {
        error(64, "[application] Failed to allocate command buffers!");
    }
}

/// Destroys the command buffers
void Application::destroy_command_buffers() {
    auto size = static_cast<u32>(command_buffers.size());
    vkFreeCommandBuffers(device.logical_device, device.command_pool, size, command_buffers.data());
    command_buffers = {};
}

/// Draws a frame
void Application::draw_frame() {
    u32 image_index;

    auto result = swapchain->acquire_next_image(&image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate_swapchain();
        return;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        error(64, "[application] Failed to acquire swap chain image!");
    }

    record_command_buffer(image_index);
    auto submit_result = swapchain->submit_command_buffers(&command_buffers[image_index], &image_index);
    if (submit_result == VK_ERROR_OUT_OF_DATE_KHR || submit_result == VK_SUBOPTIMAL_KHR || window.is_window_resized()) {
        window.clear_window_resized();
        recreate_swapchain();
        return;
    }

    if (submit_result != VK_SUCCESS) {
        error(64, "[application] Failed to submit command buffers for drawing!");
    }
}

/// Recreates the swapchain
void Application::recreate_swapchain() {
    auto extent = window.extent();

    // Pause while the window has one dimension that is sizeless (can occur during minimization)
    while (extent.width == 0 || extent.height == 0) {
        extent = window.extent();
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device.logical_device);

    if (!swapchain) {
        swapchain = std::make_unique<Swapchain>(device, extent);
    } else {
        swapchain = std::make_unique<Swapchain>(device, extent, std::move(swapchain));
        if (swapchain->image_count() != command_buffers.size()) {
            destroy_command_buffers();
            create_command_buffers();
        }
    }

    // TODO(elias): Do not create a new pipeline if the render pass is compatible
    create_pipeline();
}

/// Records the command buffer using the specified image index
void Application::record_command_buffer(u32 image_index) {
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(command_buffers[image_index], &begin_info) != VK_SUCCESS) {
        error(64, "[application] Failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = swapchain->render_pass;
    render_pass_info.framebuffer = swapchain->framebuffer_at(static_cast<s32>(image_index));
    render_pass_info.renderArea.offset = { 0, 0 };
    render_pass_info.renderArea.extent = swapchain->swapchain_extent;

    std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
    clear_values[1].depthStencil = { 1.0f, 0 };
    render_pass_info.clearValueCount = static_cast<u32>(clear_values.size());
    render_pass_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(command_buffers[image_index], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<f32>(swapchain->swapchain_extent.width);
    viewport.height = static_cast<f32>(swapchain->swapchain_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffers[image_index], 0, 1, &viewport);

    VkRect2D scissor{ { 0, 0 }, swapchain->swapchain_extent };
    vkCmdSetScissor(command_buffers[image_index], 0, 1, &scissor);

    pipeline->bind(command_buffers[image_index]);
    model->bind(command_buffers[image_index]);

    static u16 frame = 0;
    frame = (frame + 1) % 1000;

    for (u32 i = 0; i < 4; ++i) {
        PushConstantData push{};

        auto weight = static_cast<f32>(i);
        push.offset = { -0.5f + frame * 0.002f, -0.4f + 0.25f * weight };
        push.color = { 0.0f, 0.0f, 0.2f + 0.2f * weight };
        vkCmdPushConstants(command_buffers[image_index], pipeline_layout,
                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof push, &push);
        model->draw(command_buffers[image_index]);
    }

    vkCmdEndRenderPass(command_buffers[image_index]);
    if (vkEndCommandBuffer(command_buffers[image_index]) != VK_SUCCESS) {
        error(64, "[application] Failed to record command buffer!");
    }
}


}// namespace rt
