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

#include <array>

#include "renderer.h"
#include "utility.h"

namespace rt {

/// Creates a realtime renderer
Renderer::Renderer(Window &window, Device &device)
    : window{ window },
      device{ device },
      current_image_index{},
      current_frame_index{},
      frame_started{ false } {
    recreate_swapchain();
    create_command_buffers();
}

/// Destroys all renderer objects
Renderer::~Renderer() {
    destroy_command_buffers();
}

/// Retrieves the render pass of the swapchain
VkRenderPass Renderer::swapchain_render_pass() const {
    return swapchain->render_pass;
}

/// Retrieves the swapchain aspect ratio
f32 Renderer::aspect_ratio() const {
    return swapchain->extent_aspect_ratio();
}

/// Checks whether the frame is already in progress
bool Renderer::is_frame_in_progress() const {
    return frame_started;
}

/// Retrieves the current command buffer
VkCommandBuffer Renderer::current_command_buffer() const {
    assert(frame_started and "[renderer] Cannot retrieve command buffer when frame is not in progress!");
    return command_buffers[current_frame_index];
}

/// Retrieves the current frame index
u32 Renderer::frame_index() const {
    assert(frame_started and "[renderer] Cannot retrieve frame index when frame is not in progress!");
    return current_frame_index;
}

/// Begins a new frame
VkCommandBuffer Renderer::begin_frame() {
    assert(not frame_started and "[renderer] Cannot call begin_frame while already in progress!");

    auto result = swapchain->acquire_next_image(&current_image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate_swapchain();
        return nullptr;
    }

    if (result != VK_SUCCESS and result != VK_SUBOPTIMAL_KHR) {
        error(64, "[renderer] Failed to acquire swap chain image!");
    }

    frame_started = true;
    auto command_buffer = current_command_buffer();

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
        error(64, "[renderer] Failed to begin recording command buffer!");
    }

    return command_buffer;
}

/// Ends the previously started frame
void Renderer::end_frame() {
    assert(frame_started and "[renderer] Cannot call end_frame while frame is not in progress!");
    auto command_buffer = current_command_buffer();
    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
        error(64, "[renderer] Failed to record command buffer!");
    }

    auto result = swapchain->submit_command_buffers(&command_buffer, &current_image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR or result == VK_SUBOPTIMAL_KHR or window.is_window_resized()) {
        window.clear_window_resized();
        recreate_swapchain();
    } else if (result != VK_SUCCESS) {
        error(64, "[renderer] Failed to submit command buffers for drawing!");
    }

    frame_started = false;
    current_frame_index = (current_frame_index + 1) % Swapchain::MAX_FRAMES_IN_FLIGHT;
}

/// Begins a new swapchain render pass
void Renderer::begin_swapchain_render_pass(VkCommandBuffer command_buffer) const {
    assert(frame_started and "[renderer] Cannot call begin_swapchain_render_pass while frame is not in progress!");
    assert(command_buffer == current_command_buffer() and
           "[renderer] Cannot begin render pass on command buffer from a different frame!");

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = swapchain->render_pass;
    render_pass_info.framebuffer = swapchain->framebuffer_at(static_cast<s32>(current_image_index));
    render_pass_info.renderArea.offset = { 0, 0 };
    render_pass_info.renderArea.extent = swapchain->swapchain_extent;

    std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color = { { 0.01f, 0.01f, 0.01f, 1.0f } };
    clear_values[1].depthStencil = { 1.0f, 0 };
    render_pass_info.clearValueCount = static_cast<u32>(clear_values.size());
    render_pass_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<f32>(swapchain->swapchain_extent.width);
    viewport.height = static_cast<f32>(swapchain->swapchain_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor{ { 0, 0 }, swapchain->swapchain_extent };
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);
}

/// Ends the previously started swapchain render pass
void Renderer::end_swapchain_render_pass(VkCommandBuffer command_buffer) const {
    assert(frame_started and "[renderer] Cannot call end_swapchain_render_pass while frame is not in progress!");
    assert(command_buffer == current_command_buffer() and
           "[renderer] Cannot end render pass on command buffer from a different frame!");
    vkCmdEndRenderPass(command_buffer);
}

/// Creates the command buffers
void Renderer::create_command_buffers() {
    command_buffers.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = device.command_pool;
    alloc_info.commandBufferCount = static_cast<u32>(command_buffers.size());

    if (vkAllocateCommandBuffers(device.logical_device, &alloc_info, command_buffers.data()) != VK_SUCCESS) {
        error(64, "[renderer] Failed to allocate command buffers!");
    }
}

/// Destroys the command buffers
void Renderer::destroy_command_buffers() {
    auto size = static_cast<u32>(command_buffers.size());
    vkFreeCommandBuffers(device.logical_device, device.command_pool, size, command_buffers.data());
    command_buffers = {};
}

/// Recreates the swapchain
void Renderer::recreate_swapchain() {
    auto extent = window.extent();

    // Pause while the window has one dimension that is sizeless (can occur during minimization)
    while (extent.width == 0 or extent.height == 0) {
        extent = window.extent();
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device.logical_device);

    if (not swapchain) {
        swapchain = std::make_unique<Swapchain>(device, extent);
    } else {
        std::shared_ptr old = std::move(swapchain);
        swapchain = std::make_unique<Swapchain>(device, extent, old);
        if (not old->compare_swap_formats(*swapchain)) {
            error(64, "[renderer] Swapchain image (or depth) format has changed!");
        }
    }
}

}// namespace rt