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

#ifndef REALTIME_SWAPCHAIN_H
#define REALTIME_SWAPCHAIN_H

#include <vector>

#include "device.h"

namespace rt {

class Swapchain {
public:
    /// Creates a new swapchain
    /// @param device The device
    /// @param window_extent The extent of the window
    Swapchain(Device &device, VkExtent2D window_extent);

    /// Destroys all swapchain objects
    ~Swapchain();

    /// A swapchain cannot be copied
    Swapchain(const Swapchain &) = delete;
    void operator=(const Swapchain &) = delete;

    /// Retrieves the frame buffer at the specified frame index
    /// @param index The frame index
    /// @return The vulkan frame buffer at the specified frame index
    VkFramebuffer framebuffer_at(s32 index) const;

    /// Retrieves the swap chain image view at the specified frame index
    /// @param index The frame index
    /// @return The vulkan image view at the specified frame index
    VkImageView image_view_at(s32 index) const;

    /// The number of images
    /// @return The number of images
    u64 image_count() const;

    /// Calculates the aspect ratio of the swap chain extent
    /// @return The aspect ratio of the swap chain extent
    f32 extent_aspect_ratio() const;

    /// Finds a suitable format for a depth attachment
    /// @return Suitable format for a depth attachment
    VkFormat find_depth_format() const;

    /// Acquires the index of the next image to use
    /// @param image_index The index of the image
    /// @return A VkResult that indicates success
    VkResult acquire_next_image(u32 *image_index) const;

    /// Submits all command buffers (which is currently only one) and queues the current image for presentation
    ///@param buffers The command buffers
    ///@param image_index The current image index
    ///@return A VkResult that indicates success of queuing the current image for presentation
    VkResult submit_command_buffers(const VkCommandBuffer *buffers, u32 *image_index);

    /// Retrieves the swapchain extent width
    /// @return The swapchain extent width
    u32 width() const;

    /// Retrieves the swapchain extent height
    /// @return The swapchain extent height
    u32 height() const;

private:
    /// Creates the internal Vulkan swapchain
    void create_swapchain();

    /// Creates the image views
    void create_image_views();

    /// Creates the depth resources and images
    void create_depth_resources();

    /// Creates the internal Vulkan render pass. This configures a depth and color attachment.
    void create_render_pass();

    /// Creates the frame buffers
    void create_framebuffers();

    /// Creates the internal sync objects (semaphores, fences)
    void create_sync_objects();

    /// Helper function for calculating the extent of the swapchain from the surface capabilities
    /// @param capabilities The surface capabilities
    /// @return The calculated extent
    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities) const;

    /// Our list of friends :)
    friend class Application;

    Device &device;
    VkExtent2D window_extent;
    VkSwapchainKHR swapchain;
    VkFormat swapchain_image_format;
    VkExtent2D swapchain_extent;

    std::vector<VkFramebuffer> swapchain_framebuffers;
    VkRenderPass render_pass;

    std::vector<VkImage> depth_images;
    std::vector<VkDeviceMemory> depth_image_memorys;
    std::vector<VkImageView> depth_image_views;
    std::vector<VkImage> swapchain_images;
    std::vector<VkImageView> swapchain_image_views;

    std::vector<VkSemaphore> image_available_semaphores;
    std::vector<VkSemaphore> render_finished_semaphores;
    std::vector<VkFence> in_flight_fences;
    std::vector<VkFence> images_in_flight;
    u64 current_frame;
};

}// namespace rt

#endif// REALTIME_SWAPCHAIN_H
