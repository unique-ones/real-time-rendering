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
#include <set>
#include <unordered_set>

#include "device.h"
#include "utility.h"

namespace rt {

namespace {

constexpr std::array VALIDATION_LAYERS = { "VK_LAYER_KHRONOS_validation" };
constexpr std::array DEVICE_EXTENSIONS = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

/// Debug callback for Vulkan messages
/// @param data The actual message data
VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT,
                                                     VkDebugUtilsMessageTypeFlagsEXT,
                                                     const VkDebugUtilsMessengerCallbackDataEXT *data,
                                                     void *) {
    std::fprintf(stderr, "Validation layer: %s\n", data->pMessage);
    return VK_FALSE;
}

/// Creates a Vulkan debug utils messenger, if the facility is available
/// @param instance The current vulkan instance
/// @param create_info The configuration for the messenger
/// @param allocator The allocator
/// @param messenger The actual messenger
/// @return A VkResult that indicates success
VkResult vulkan_create_debug_utils_messenger(VkInstance instance,
                                             const VkDebugUtilsMessengerCreateInfoEXT *create_info,
                                             const VkAllocationCallbacks *allocator,
                                             VkDebugUtilsMessengerEXT *messenger) {
    using Sig = PFN_vkCreateDebugUtilsMessengerEXT;
    if (auto *f = reinterpret_cast<Sig>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"))) {
        return f(instance, create_info, allocator, messenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

/// Destroys the Vulkan debug utils messenger, if the facility is available
/// @param instance The current vulkan instance
/// @param messenger The messenger
/// @param allocator The allocator
void vulkan_destroy_debug_utils_messenger(VkInstance instance,
                                          VkDebugUtilsMessengerEXT messenger,
                                          const VkAllocationCallbacks *allocator) {
    using Sig = PFN_vkDestroyDebugUtilsMessengerEXT;
    if (auto *f = reinterpret_cast<Sig>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"))) {
        f(instance, messenger, allocator);
    }
}

/// Checks if there is support for validation layers
/// @return A boolean value that indicates support for validation layers
bool vulkan_validation_layer_support() {
    u32 count;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> layers(count);
    vkEnumerateInstanceLayerProperties(&count, layers.data());

    for (const auto &layer_name : VALIDATION_LAYERS) {
        auto found = false;
        for (const auto &properties : layers) {
            if (std::string_view(layer_name) == properties.layerName) {
                found = true;
                break;
            }
        }
        if (not found) {
            return false;
        }
    }
    return true;
}

/// Returns the required instance extensions for the GLFW context
/// @return A list of required extensions
std::vector<const char *> vulkan_required_extensions() {
    u32 count;
    auto glfw_extensions = glfwGetRequiredInstanceExtensions(&count);

    std::vector<const char *> extensions{ glfw_extensions, glfw_extensions + count };
    if constexpr (DeviceValidation) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

/// Validates that the required Vulkan extensions are actually present
void vulkan_validate_required_extensions() {
    u32 count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> extensions(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());

    std::unordered_set<std::string> available{};
    for (const auto &extension : extensions) {
        available.insert(extension.extensionName);
    }

    for (auto required_extensions = vulkan_required_extensions(); const auto &required : required_extensions) {
        if (not available.contains(required)) {
            error(64, "[device] Missing required Vulkan extension.");
        }
    }
}

/// Fills the create info for the Vulkan debug messenger
/// @param info The info struct
void vulkan_fill_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &info) {
    info = {};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    info.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    info.pfnUserCallback = vulkan_debug_callback;
    info.pUserData = nullptr;
}

/// Checks whether the Vulkan device has support for all device extensions
/// @param device The physical device
/// @return A boolean value that indicates support for the required device extensions
bool vulkan_device_extension_support(VkPhysicalDevice device) {
    u32 count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> extensions(count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, extensions.data());
    std::set<std::string> required_extensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());
    for (const auto &extension : extensions) {
        required_extensions.erase(extension.extensionName);
    }
    return required_extensions.empty();
}

}// namespace

/// Checks whether the queue family indices are complete
bool QueueFamilyIndices::complete() const {
    return graphics_family and present_family;
}

/// Creates a device for the specified window
Device::Device(Window &window)
    : window{ window },
      instance{},
      messenger{},
      physical_device{ VK_NULL_HANDLE },
      physical_device_properties{},
      command_pool{},
      logical_device{},
      surface{},
      graphics_queue{},
      present_queue{} {
    create_instance();
    create_messenger();
    create_surface();
    pick_physical_device();
    create_logical_device();
    create_command_pool();
}

/// Destroys the device
Device::~Device() {
    vkDestroyCommandPool(logical_device, command_pool, nullptr);
    vkDestroyDevice(logical_device, nullptr);

    if constexpr (DeviceValidation) {
        vulkan_destroy_debug_utils_messenger(instance, messenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}

/// Checks for swapchain support of the current device
SwapchainDetails Device::swapchain_support() const {
    return swapchain_support(physical_device);
}

/// Checks if the current device has the specified type of memory
u32 Device::find_memory_type(u32 filter, VkMemoryPropertyFlags props) const {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
    for (u32 i = 0; i < memory_properties.memoryTypeCount; i++) {
        if ((filter & (1 << i)) and (memory_properties.memoryTypes[i].propertyFlags & props) == props) {
            return i;
        }
    }

    error(64, "[device] Failed to find suitable memory type for device!");
}

/// Find the supported format among a list of candidates
VkFormat Device::find_supported_format(const std::vector<VkFormat> &candidates,
                                       VkImageTiling tiling,
                                       VkFormatFeatureFlags features) const {
    for (auto format : candidates) {
        VkFormatProperties format_properties{};
        vkGetPhysicalDeviceFormatProperties(physical_device, format, &format_properties);
        if (tiling == VK_IMAGE_TILING_LINEAR and (format_properties.linearTilingFeatures & features) == features) {
            return format;
        }
        if (tiling == VK_IMAGE_TILING_OPTIMAL and (format_properties.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    error(64, "[device] Failed to find suitable format!");
}

/// Creates the Vulkan instance
void Device::create_instance() {
    if constexpr (DeviceValidation) {
        if (not vulkan_validation_layer_support()) {
            error(64, "[device] Validation layers were requested, but are not supported!");
        }
    }

    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = window.spec.name.c_str();
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "Real-Time Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    // Fetch the required extensions
    auto extensions = vulkan_required_extensions();

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = static_cast<u32>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
    if constexpr (DeviceValidation) {
        create_info.enabledLayerCount = static_cast<u32>(VALIDATION_LAYERS.size());
        create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();

        vulkan_fill_debug_messenger_create_info(debug_create_info);
        create_info.pNext = &debug_create_info;
    } else {
        create_info.enabledLayerCount = 0;
        create_info.pNext = nullptr;
    }

    if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
        error(64, "[device] Could not create Vulkan instance!");
    }

    vulkan_validate_required_extensions();
}

/// Creates a debug messenger if validation is turned on
void Device::create_messenger() {
    if constexpr (not DeviceValidation) {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT create_info{};
    vulkan_fill_debug_messenger_create_info(create_info);
    if (vulkan_create_debug_utils_messenger(instance, &create_info, nullptr, &messenger) != VK_SUCCESS) {
        error(64, "[device] Failed to setup debug messenger!");
    }
}

/// Creates the Vulkan draw surface
void Device::create_surface() {
    window.create_surface(instance, &surface);
}

/// Chooses a suitable physical device (graphics card)
void Device::pick_physical_device() {
    u32 device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    if (device_count == 0) {
        error(64, "[device] Failed to find GPU with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

    for (const auto &device : devices) {
        if (is_device_suitable(device)) {
            physical_device = device;
            break;
        }
    }

    if (physical_device == VK_NULL_HANDLE) {
        error(64, "[device] Failed to find GPU with Vulkan support!");
    }

    vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);
}

/// Creates a logical Vulkan device
void Device::create_logical_device() {
    auto indices = find_queue_families(physical_device);
    auto graphics_family = indices.graphics_family.value_or(0);
    auto present_family = indices.present_family.value_or(0);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<u32> queue_families = { graphics_family, present_family };

    for (auto queue_priority = 1.0f; auto queue_family : queue_families) {
        VkDeviceQueueCreateInfo queue_create_info = {};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures device_features = {};
    device_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = static_cast<u32>(queue_create_infos.size());
    create_info.pQueueCreateInfos = queue_create_infos.data();
    create_info.pEnabledFeatures = &device_features;
    create_info.enabledExtensionCount = static_cast<u32>(DEVICE_EXTENSIONS.size());
    create_info.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

    if (vkCreateDevice(physical_device, &create_info, nullptr, &logical_device) != VK_SUCCESS) {
        error(64, "[device] Failed to create logical Vulkan device!");
    }

    vkGetDeviceQueue(logical_device, graphics_family, 0, &graphics_queue);
    vkGetDeviceQueue(logical_device, present_family, 0, &present_queue);
}

/// Creates a command pool
void Device::create_command_pool() {
    auto indices = find_queue_families(physical_device);

    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = indices.graphics_family.value_or(0);
    pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(logical_device, &pool_info, nullptr, &command_pool) != VK_SUCCESS) {
        error(64, "[device] Failed to create Vulkan command pool!");
    }
}

/// Checks whether a physical device is suitable for use
bool Device::is_device_suitable(VkPhysicalDevice device) const {
    auto indices = find_queue_families(device);
    auto extensions_supported = vulkan_device_extension_support(device);
    auto swapchain_adequate = false;

    if (extensions_supported) {
        auto [_, formats, present_modes] = swapchain_support(device);
        swapchain_adequate = !formats.empty() and !present_modes.empty();
    }

    VkPhysicalDeviceFeatures supported_features;
    vkGetPhysicalDeviceFeatures(device, &supported_features);
    return indices.complete() and extensions_supported and swapchain_adequate and supported_features.samplerAnisotropy;
}

/// Finds the queue family indices for the specified physical device
QueueFamilyIndices Device::find_queue_families(VkPhysicalDevice device) const {
    u32 queue_familiy_count;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_familiy_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_familiy_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_familiy_count, queue_families.data());

    QueueFamilyIndices indices{};
    for (s32 idx = 0; const auto &family : queue_families) {
        if (family.queueCount > 0 and family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family = idx;
        }

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, idx, surface, &present_support);
        if (family.queueCount > 0 and present_support) {
            indices.present_family = idx;
        }
        if (indices.complete()) {
            break;
        }

        idx++;
    }
    return indices;
}

/// Checks for swapchain support of the specified device
SwapchainDetails Device::swapchain_support(VkPhysicalDevice device) const {
    SwapchainDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    u32 format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
    if (format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());
    }

    u32 present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);
    if (present_mode_count != 0) {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes.data());
    }

    return details;
}

/// Creates a buffer
void Device::create_buffer(VkDeviceSize size,
                           VkBufferUsageFlags usage,
                           VkMemoryPropertyFlags props,
                           VkBuffer &buffer,
                           VkDeviceMemory &buffer_memory) const {
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(logical_device, &buffer_info, nullptr, &buffer) != VK_SUCCESS) {
        error(64, "[device] Failed to create buffer!");
    }

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(logical_device, buffer, &requirements);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = requirements.size;
    alloc_info.memoryTypeIndex = find_memory_type(requirements.memoryTypeBits, props);
    if (vkAllocateMemory(logical_device, &alloc_info, nullptr, &buffer_memory) != VK_SUCCESS) {
        error(64, "[device] Failed to allocate buffer memory!");
    }

    vkBindBufferMemory(logical_device, buffer, buffer_memory, 0);
}

/// Begins single time commands
VkCommandBuffer Device::begin_commands() const {
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = command_pool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(logical_device, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &begin_info);
    return command_buffer;
}

/// Ends the single time commands for the command buffer
void Device::end_commands(VkCommandBuffer command_buffer) const {
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue);
    vkFreeCommandBuffers(logical_device, command_pool, 1, &command_buffer);
}

/// Copes the source buffer to the destination buffer
void Device::copy_buffer(VkBuffer source, VkBuffer destination, VkDeviceSize size) const {
    auto cmd = begin_commands();
    VkBufferCopy copy_region{};
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = size;
    vkCmdCopyBuffer(cmd, source, destination, 1, &copy_region);
    end_commands(cmd);
}

/// Copies a buffer to the specified image
void Device::copy_buffer_to_image(VkBuffer buffer, VkImage image, u32 width, u32 height, u32 layer_count) const {
    auto cmd = begin_commands();
    VkBufferImageCopy copy_region{};
    copy_region.bufferOffset = 0;
    copy_region.bufferRowLength = 0;
    copy_region.bufferImageHeight = 0;
    copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.imageSubresource.mipLevel = 0;
    copy_region.imageSubresource.baseArrayLayer = 0;
    copy_region.imageSubresource.layerCount = layer_count;
    copy_region.imageOffset = { 0, 0, 0 };
    copy_region.imageExtent = { width, height, 1 };
    vkCmdCopyBufferToImage(cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
    end_commands(cmd);
}

/// Creates an image from the specified create-info
void Device::create_image(const VkImageCreateInfo &info,
                          VkMemoryPropertyFlags props,
                          VkImage &image,
                          VkDeviceMemory &memory) const {
    if (vkCreateImage(logical_device, &info, nullptr, &image) != VK_SUCCESS) {
        error(64, "[device] Failed to create Vulkan image!");
    }

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(logical_device, image, &memory_requirements);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = memory_requirements.size;
    alloc_info.memoryTypeIndex = find_memory_type(memory_requirements.memoryTypeBits, props);

    if (vkAllocateMemory(logical_device, &alloc_info, nullptr, &memory) != VK_SUCCESS) {
        error(64, "[device] Failed to allocate image memory!");
    }
    if (vkBindImageMemory(logical_device, image, memory, 0) != VK_SUCCESS) {
        error(64, "[device] Failed to bind image memory!");
    }
}

}// namespace rt
