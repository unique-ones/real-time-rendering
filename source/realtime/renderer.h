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

#ifndef REALTIME_RENDERER_H
#define REALTIME_RENDERER_H

#include <memory>
#include <vector>

#include "device.h"
#include "swapchain.h"
#include "window.h"

namespace rt {

class Renderer {
public:
    /// Creates a realtime renderer
    /// @param window The window handle
    /// @param device The device handle
    Renderer(Window &window, Device &device);

    /// Destroys all renderer objects
    ~Renderer();

    /// An renderer cannot be copied or moved
    Renderer(const Renderer &) = delete;
    Renderer &operator=(const Renderer &) = delete;
    Renderer(Renderer &&) = delete;
    Renderer &&operator=(Renderer &&) = delete;

    /// Retrieves the render pass of the swapchain
    /// @return The render pass of the swapchain
    VkRenderPass swapchain_render_pass() const;

    /// Retrieves the swapchain aspect ratio
    /// @return The swapchain aspect ratio
    f32 aspect_ratio() const;

    /// Checks whether the frame is already in progress
    /// @return A value that indicates whether the frame is already in progress
    bool is_frame_in_progress() const;

    /// Retrieves the current command buffer
    /// @return The current command buffer
    VkCommandBuffer current_command_buffer() const;

    /// Retrieves the current frame index
    /// @return The current frame index
    u32 frame_index() const;

    /// Begins a new frame
    /// @return The recording command buffer
    VkCommandBuffer begin_frame();

    /// Ends the previously started frame
    void end_frame();

    /// Begins a new swapchain render pass
    /// @param command_buffer The recording command buffer
    void begin_swapchain_render_pass(VkCommandBuffer command_buffer);

    /// Ends the previously started swapchain render pass
    /// @param command_buffer The recording command buffer
    void end_swapchain_render_pass(VkCommandBuffer command_buffer);

private:
    /// Creates the command buffers
    void create_command_buffers();

    /// Destroys the command buffers
    void destroy_command_buffers();

    /// Recreates the swapchain
    void recreate_swapchain();

    Window &window;
    Device &device;
    std::unique_ptr<Swapchain> swapchain;
    std::vector<VkCommandBuffer> command_buffers;
    u32 current_image_index;
    u32 current_frame_index;
    bool frame_started;
};

}// namespace rt

#endif// REALTIME_RENDERER_H
