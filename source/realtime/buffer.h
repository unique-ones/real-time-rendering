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

#ifndef REALTIME_BUFFER_H
#define REALTIME_BUFFER_H

#include "device.h"

namespace rt {

struct MappedRange {
    VkDeviceSize size = VK_WHOLE_SIZE;
    VkDeviceSize offset = 0;
};

struct Buffer {
    Device &device;
    void *mapped = nullptr;
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;

    VkDeviceSize buffer_size;
    u32 instance_count;
    VkDeviceSize instance_size;
    VkDeviceSize alignment_size;
    VkBufferUsageFlags usage_flags;
    VkMemoryPropertyFlags memory_property_flags;


    /// Creates a new Vulkan buffer
    /// @param device The device
    /// @param instance_size The size of the buffer
    /// @param instance_count The number of instances
    /// @param memory_property_flags The memory properties
    /// @param min_offset_alignment The minimal offset alignment
    /// @return A new buffer instance
    Buffer(Device &device,
           VkDeviceSize instance_size,
           u32 instance_count,
           VkBufferUsageFlags usage_flags,
           VkMemoryPropertyFlags memory_property_flags,
           VkDeviceSize min_offset_alignment = 1);

    /// Destroys the buffer and all its related data
    ~Buffer();

    /// A buffer may not be copied
    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;

    /// Maps a memory range of this buffer. If successful, maps points to the specified buffer range
    /// @param range The buffer range
    /// @return A result variable that indicates success
    VkResult map(const MappedRange &range = {});

    /// Unmaps a memory range
    void unmap();

    /// Writes the specified data to the mapped buffer.
    /// @param data The data
    /// @param range The buffer range
    void write(void *data, const MappedRange &range = {});

    /// Flushes the mapped memory range of the buffer to make it visible to the device
    /// @param range The buffer range
    /// @return A result variable that indicates success
    VkResult flush(const MappedRange &range = {});

    /// Invalidates the mapped memory range of the buffer to make it visible to the host
    /// @param range The buffer range
    /// @return A result variable that indicates success
    VkResult invalidate(const MappedRange &range = {});

    /// Retrieves the descriptor info
    /// @param range The buffer range
    /// @return The descriptor buffer info
    VkDescriptorBufferInfo descriptor_info(const MappedRange &range = {});


    /// Writes to the specified index
    /// @param data The data
    /// @param index The index
    void write(void *data, s32 index);

    /// Flushes the buffer at the specified index
    /// @param index The index
    /// @return A result variable that indicates success
    VkResult flush(s32 index);

    /// Invalidates the buffer at the specified index
    /// @param index The index
    /// @return A result variable that indicates success
    VkResult invalidate(s32 index);

    /// Retrieves the descriptor info at the specified index
    /// @param index The index
    /// @return The descriptor buffer info
    VkDescriptorBufferInfo descriptor_info(int index);

private:
    /// Returns the minimum instance size required to be cmpatible with devices min_offset_alignment
    /// @param instance_size The size of an instance
    /// @param min_offset_alignment The minimum required alignment, in bytes, for the offset member
    /// @return A result variable that indicates success
    static VkDeviceSize alignment(VkDeviceSize instance_size, VkDeviceSize min_offset_alignment);
};
}// namespace rt

#endif// REALTIME_BUFFER_H
