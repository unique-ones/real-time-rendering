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

#ifndef REALTIME_DEVICE_H
#define REALTIME_DEVICE_H

#include <optional>
#include <vector>

#include "window.h"

namespace rt {

#ifdef NDEBUG
constexpr static inline auto DeviceValidation = false;
#else
constexpr static inline auto DeviceValidation = true;
#endif

struct SwapchainDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

struct QueueFamilyIndices {
    std::optional<u32> graphics_family;
    std::optional<u32> present_family;

    /// Checks whether the queue family indices are complete
    /// @return A boolean value that indicates completeness
    bool complete() const;
};

class Device {
public:
    /// Creates a device for the specified window
    /// @param window The window
    explicit Device(Window &window);

    /// Destroys the device
    ~Device();

    /// A device cannot be copied or moved
    Device(const Device &d) = delete;
    Device &operator=(const Device &d) = delete;
    Device(Device &&d) = delete;
    Device &operator=(Device &&d) = delete;

    /// Checks for swapchain support of the current device
    /// @return Swapchain support details
    SwapchainDetails swapchain_support() const;

    /// Checks if the current device has the specified type of memory
    /// @param filter The filter
    /// @param props The memory properties
    /// @return ...
    u32 find_memory_type(u32 filter, VkMemoryPropertyFlags props) const;

    /// Find the supported format among a list of candidates
    /// @param candidates The candidates
    /// @param tiling The image tiling
    /// @param features The format feature flags
    /// @return The supported format
    VkFormat find_supported_format(const std::vector<VkFormat> &candidates,
                                   VkImageTiling tiling,
                                   VkFormatFeatureFlags features) const;

    /// Creates a buffer
    /// @param size The size of the buffer
    /// @param usage The buffer usage flags
    /// @param props The memory properties
    /// @param buffer The actual buffer
    /// @param buffer_memory The buffer memory
    void create_buffer(VkDeviceSize size,
                       VkBufferUsageFlags usage,
                       VkMemoryPropertyFlags props,
                       VkBuffer &buffer,
                       VkDeviceMemory &buffer_memory) const;

    /// Begins single time commands
    /// @return A command buffer for commands
    VkCommandBuffer begin_commands() const;

    /// Ends the single time commands for the command buffer
    /// @param command_buffer The command buffer
    void end_commands(VkCommandBuffer command_buffer) const;

    /// Copes the source buffer to the destination buffer
    /// @param source The source buffer
    /// @param destination The destination buffer
    /// @param size The size of the buffers
    void copy_buffer(VkBuffer source, VkBuffer destination, VkDeviceSize size) const;

    /// Copies a buffer to the specified image
    /// @param buffer The buffer
    /// @param image The image where the buffer is copied to
    /// @param width The width of the image
    /// @param height The height of the image
    /// @param layer_count The number of layers
    void copy_buffer_to_image(VkBuffer buffer, VkImage image, u32 width, u32 height, u32 layer_count) const;

    /// Creates an image from the specified create-info
    /// @param info The create information
    /// @param props The memory properties for the image
    /// @param image The actual image
    /// @param memory The memory that holds the image
    void create_image(const VkImageCreateInfo &info,
                      VkMemoryPropertyFlags props,
                      VkImage &image,
                      VkDeviceMemory &memory) const;

private:
    /// Creates the Vulkan instance
    void create_instance();

    /// Creates a debug messenger if validation is turned on
    void create_messenger();

    /// Creates the Vulkan draw surface
    void create_surface();

    /// Chooses a suitable physical device (graphics card)
    void pick_physical_device();

    /// Creates a logical Vulkan device
    void create_logical_device();

    /// Creates a command pool
    void create_command_pool();

    /// Checks whether a physical device is suitable for use
    /// @param device The physical device
    /// @return A boolean value that indicates whether a device is suitable or not
    bool is_device_suitable(VkPhysicalDevice device) const;

    /// Finds the queue family indices for the specified physical device
    /// @param device The physical device
    /// @return The queue family indices
    QueueFamilyIndices find_queue_families(VkPhysicalDevice device) const;

    /// Checks for swapchain support of the specified device
    /// @param device The physical device
    /// @return The swapchain support details
    SwapchainDetails swapchain_support(VkPhysicalDevice device) const;

    /// List of friends :)
    friend class Pipeline;
    friend class Swapchain;
    friend class Application;
    friend class Model;

    Window &window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT messenger;
    VkPhysicalDevice physical_device;
    VkPhysicalDeviceProperties physical_device_properties;
    VkCommandPool command_pool;

    VkDevice logical_device;
    VkSurfaceKHR surface;
    VkQueue graphics_queue;
    VkQueue present_queue;
};

}// namespace rt

#endif// REALTIME_DEVICE_H
