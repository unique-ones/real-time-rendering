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
#include <limits>

#include "swapchain.h"
#include "utility.h"

namespace rt {

namespace {

/// Chooses a suitable surface format from a list of formats
/// @param formats The formats to choose from
/// @return A suitable surface format
VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR> &formats) {
    for (const auto &format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB and format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }
    return formats[0];
}

/// Chooses a suitable present mode from a list of present modes
/// @param modes The present modes to choose from
/// @return A suitable present mode (MAILBOX)
VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR> &modes) {
    if constexpr (not Window::VERTICAL_SYNC) {
        for (const auto &mode : modes) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return mode;
            }
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

}// namespace

/// Creates a new swapchain
Swapchain::Swapchain(Device &device, VkExtent2D window_extent)
    : device{ device },
      window_extent{ window_extent },
      swapchain{},
      swapchain_image_format{},
      swapchain_depth_format{},
      swapchain_extent{},
      render_pass{},
      current_frame{} {
    init();
}

/// Creates a new swapchain by reusing resources of a previous swapchain
Swapchain::Swapchain(Device &device, VkExtent2D window_extent, std::shared_ptr<Swapchain> previous)
    : device{ device },
      window_extent{ window_extent },
      swapchain{},
      previous{ previous },
      swapchain_image_format{},
      swapchain_depth_format{},
      swapchain_extent{},
      render_pass{},
      current_frame{} {
    init();
    previous.reset();
}

/// Destroys all swapchain objects
Swapchain::~Swapchain() {
    for (auto image_view : swapchain_image_views) {
        vkDestroyImageView(device.logical_device, image_view, nullptr);
    }
    if (swapchain) {
        vkDestroySwapchainKHR(device.logical_device, swapchain, nullptr);
        swapchain = nullptr;
    }
    for (decltype(depth_images)::size_type i = 0; i < depth_images.size(); ++i) {
        vkDestroyImageView(device.logical_device, depth_image_views[i], nullptr);
        vkDestroyImage(device.logical_device, depth_images[i], nullptr);
        vkFreeMemory(device.logical_device, depth_image_memorys[i], nullptr);
    }
    for (auto framebuffer : swapchain_framebuffers) {
        vkDestroyFramebuffer(device.logical_device, framebuffer, nullptr);
    }
    vkDestroyRenderPass(device.logical_device, render_pass, nullptr);
    for (s32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroySemaphore(device.logical_device, render_finished_semaphores[i], nullptr);
        vkDestroySemaphore(device.logical_device, image_available_semaphores[i], nullptr);
        vkDestroyFence(device.logical_device, in_flight_fences[i], nullptr);
    }
}

/// Retrieves the frame buffer at the specified frame index
VkFramebuffer Swapchain::framebuffer_at(s32 index) const {
    return swapchain_framebuffers[index];
}

/// Retrieves the swap chain image view at the specified frame index
VkImageView Swapchain::image_view_at(s32 index) const {
    return swapchain_image_views[index];
}

/// The number of images
u64 Swapchain::image_count() const {
    return swapchain_images.size();
}

/// Calculates the aspect ratio of the swap chain extent
f32 Swapchain::extent_aspect_ratio() const {
    return static_cast<f32>(swapchain_extent.width) / static_cast<f32>(swapchain_extent.height);
}

/// Finds a suitable format for a depth attachment
VkFormat Swapchain::find_depth_format() const {
    return device.find_supported_format(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

/// Acquires the index of the next image to use
VkResult Swapchain::acquire_next_image(u32 *image_index) const {
    vkWaitForFences(device.logical_device, 1, &in_flight_fences[current_frame], VK_TRUE,
                    std::numeric_limits<u64>::max());

    /// NOTE(elias): The image available semaphore must not be signaled
    return vkAcquireNextImageKHR(device.logical_device, swapchain, std::numeric_limits<uint64_t>::max(),
                                 image_available_semaphores[current_frame], VK_NULL_HANDLE, image_index);
}

/// Submits all command buffers (which is currently only one) and queues the current image for presentation
VkResult Swapchain::submit_command_buffers(const VkCommandBuffer *buffers, u32 *image_index) {
    if (images_in_flight[*image_index] != VK_NULL_HANDLE) {
        vkWaitForFences(device.logical_device, 1, &images_in_flight[*image_index], VK_TRUE, UINT64_MAX);
    }
    images_in_flight[*image_index] = in_flight_fences[current_frame];

    std::array<VkPipelineStageFlags, 1> wait_stages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    std::array<VkSemaphore, 1> wait_semaphores = { image_available_semaphores[current_frame] };
    std::array<VkSemaphore, 1> signal_semaphores = { render_finished_semaphores[current_frame] };

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores.data();
    submit_info.pWaitDstStageMask = wait_stages.data();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = buffers;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores.data();

    vkResetFences(device.logical_device, 1, &in_flight_fences[current_frame]);
    if (vkQueueSubmit(device.graphics_queue, 1, &submit_info, in_flight_fences[current_frame]) != VK_SUCCESS) {
        error(64, "[swapchain] Failed to submit draw command buffers!");
    }

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores.data();
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &swapchain;
    present_info.pImageIndices = image_index;

    auto result = vkQueuePresentKHR(device.present_queue, &present_info);
    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    return result;
}

/// Retrieves the swapchain extent width
u32 Swapchain::width() const {
    return swapchain_extent.width;
}

/// Retrieves the swapchain extent height
u32 Swapchain::height() const {
    return swapchain_extent.height;
}

/// Compares the formats of the current swapchain to the ones of an other swapchain
bool Swapchain::compare_swap_formats(const Swapchain &other) const {
    return swapchain_depth_format == other.swapchain_depth_format and
           swapchain_image_format == other.swapchain_image_format;
}

/// Initializes the swapchain
void Swapchain::init() {
    create_swapchain();
    create_image_views();
    create_render_pass();
    create_depth_resources();
    create_framebuffers();
    create_sync_objects();
}

/// Creates the internal Vulkan swapchain
void Swapchain::create_swapchain() {
    auto [capabilities, formats, present_modes] = device.swapchain_support();
    auto [format, color_space] = choose_swap_surface_format(formats);
    auto present_mode = choose_swap_present_mode(present_modes);
    auto extent = choose_swap_extent(capabilities);

    auto image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 and image_count > capabilities.maxImageCount) {
        image_count = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = device.surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = format;
    create_info.imageColorSpace = color_space;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    auto [graphics_family, present_family] = device.find_queue_families(device.physical_device);
    if (u32 queue_family_indices[] = { *graphics_family, *present_family }; graphics_family != present_family) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
    }

    create_info.preTransform = capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = previous ? previous->swapchain : VK_NULL_HANDLE;

    if (auto result = vkCreateSwapchainKHR(device.logical_device, &create_info, nullptr, &swapchain);
        result != VK_SUCCESS) {
        error(64, "[swapchain] Failed to create swapchain!");
    }

    // We only specified a minimum number of images in the swap chain, so the implementation is
    // allowed to create a swapchain with more. That's why we'll first query the final number of
    // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
    // retrieve the handles.
    vkGetSwapchainImagesKHR(device.logical_device, swapchain, &image_count, nullptr);
    swapchain_images.resize(image_count);
    vkGetSwapchainImagesKHR(device.logical_device, swapchain, &image_count, swapchain_images.data());
    swapchain_image_format = format;
    swapchain_extent = extent;
}

/// Creates the image views
void Swapchain::create_image_views() {
    swapchain_image_views.resize(swapchain_images.size());
    for (u64 i = 0; i < swapchain_images.size(); ++i) {
        VkImageViewCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.image = swapchain_images[i];
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.format = swapchain_image_format;
        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        info.subresourceRange.baseMipLevel = 0;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device.logical_device, &info, nullptr, &swapchain_image_views[i]) != VK_SUCCESS) {
            error(64, "[swapchain] Failed to create texture image view!");
        }
    }
}

/// Creates the depth resources and images
void Swapchain::create_depth_resources() {
    auto depth_format = find_depth_format();
    swapchain_depth_format = depth_format;
    depth_images.resize(image_count());
    depth_image_memorys.resize(image_count());
    depth_image_views.resize(image_count());

    for (decltype(depth_images)::size_type i = 0; i < depth_images.size(); ++i) {
        VkImageCreateInfo image_info{};
        image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.imageType = VK_IMAGE_TYPE_2D;
        image_info.extent.width = swapchain_extent.width;
        image_info.extent.height = swapchain_extent.height;
        image_info.extent.depth = 1;
        image_info.mipLevels = 1;
        image_info.arrayLayers = 1;
        image_info.format = depth_format;
        image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        image_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_info.flags = 0;

        device.create_image(image_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depth_images[i], depth_image_memorys[i]);

        VkImageViewCreateInfo view_info{};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = depth_images[i];
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = depth_format;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device.logical_device, &view_info, nullptr, &depth_image_views[i]) != VK_SUCCESS) {
            error(64, "[swapchain] Failed to create texture image view!");
        }
    }
}

/// Creates the internal Vulkan render pass. This configures a depth and color attachment.
void Swapchain::create_render_pass() {
    VkAttachmentDescription depth_attachment{};
    depth_attachment.format = find_depth_format();
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription color_attachment{};
    color_attachment.format = swapchain_image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    VkSubpassDependency dependency{};
    dependency.dstSubpass = 0;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array attachments = { color_attachment, depth_attachment };
    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = static_cast<u32>(attachments.size());
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    if (vkCreateRenderPass(device.logical_device, &render_pass_info, nullptr, &render_pass) != VK_SUCCESS) {
        error(64, "[swapchain] Failed to create render pass!");
    }
}

/// Creates the frame buffers
void Swapchain::create_framebuffers() {
    swapchain_framebuffers.resize(image_count());
    for (u64 i = 0; i < image_count(); i++) {
        std::array attachments = { swapchain_image_views[i], depth_image_views[i] };

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = render_pass;
        framebuffer_info.attachmentCount = static_cast<u32>(attachments.size());
        framebuffer_info.pAttachments = attachments.data();
        framebuffer_info.width = swapchain_extent.width;
        framebuffer_info.height = swapchain_extent.height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(device.logical_device, &framebuffer_info, nullptr, &swapchain_framebuffers[i]) !=
            VK_SUCCESS) {
            error(64, "[swapchain] Failed to create framebuffer!");
        }
    }
}

/// Creates the internal sync objects (semaphores, fences)
void Swapchain::create_sync_objects() {
    image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
    images_in_flight.resize(image_count(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (u64 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device.logical_device, &semaphore_info, nullptr, &image_available_semaphores[i]) !=
                    VK_SUCCESS or
            vkCreateSemaphore(device.logical_device, &semaphore_info, nullptr, &render_finished_semaphores[i]) !=
                    VK_SUCCESS or
            vkCreateFence(device.logical_device, &fence_info, nullptr, &in_flight_fences[i]) != VK_SUCCESS) {
            error(64, "[swapchain] Failed to create synchronization objects!");
        }
    }
}

/// Helper function for calculating the extent of the swapchain from the surface capabilities
VkExtent2D Swapchain::choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities) const {
    if (capabilities.currentExtent.width != std::numeric_limits<u32>::max()) {
        return capabilities.currentExtent;
    }

    auto extent = window_extent;
    extent.width =
            std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, extent.width));
    extent.height =
            std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, extent.height));

    return extent;
}

}// namespace rt
