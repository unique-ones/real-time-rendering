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

#include "buffer.h"

#include <cassert>
#include <cstring>
#include "vulkan/vulkan_core.h"

namespace rt {

/// Creates a new Vulkan buffer
Buffer::Buffer(Device &device,
               VkDeviceSize instance_size,
               u32 instance_count,
               VkBufferUsageFlags usage_flags,
               VkMemoryPropertyFlags memory_property_flags,
               VkDeviceSize min_offset_alignment)
    : device{ device },
      instance_count{ instance_count },
      instance_size{ instance_size },
      usage_flags{ usage_flags },
      memory_property_flags{ memory_property_flags } {
    alignment_size = alignment(instance_size, min_offset_alignment);
    buffer_size = alignment_size * instance_count;
    device.create_buffer(buffer_size, usage_flags, memory_property_flags, buffer, memory);
}

/// Destroys the buffer and all its related data
Buffer::~Buffer() {
    unmap();
    vkDestroyBuffer(device.logical_device, buffer, nullptr);
    vkFreeMemory(device.logical_device, memory, nullptr);
}

/// Maps a memory range of this buffer. If successful, maps points to the specified buffer range
VkResult Buffer::map(const MappedRange &range) {
    assert(buffer and memory and "[buffer] Called map on buffer before create!");
    return vkMapMemory(device.logical_device, memory, range.offset, range.size, 0, &mapped);
}

/// Unmaps a memory range
void Buffer::unmap() {
    if (mapped) {
        vkUnmapMemory(device.logical_device, memory);
        mapped = nullptr;
    }
}

/// Writes the specified data to the mapped buffer.
void Buffer::write(void *data, const MappedRange &range) {
    assert(mapped and "[buffer] Cannot copy to unmapped buffer!");

    if (range.size == VK_WHOLE_SIZE) {
        std::memcpy(mapped, data, buffer_size);
    } else {
        auto *memory_offset = reinterpret_cast<char *>(mapped);
        memory_offset += range.offset;
        std::memcpy(memory_offset, data, range.size);
    }
}

/// Flushes the mapped memory range of the buffer to make it visible to the device
VkResult Buffer::flush(const MappedRange &range) {
    VkMappedMemoryRange mapped_range{};
    mapped_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mapped_range.memory = memory;
    mapped_range.offset = range.offset;
    mapped_range.size = range.size;
    return vkFlushMappedMemoryRanges(device.logical_device, 1, &mapped_range);
}

/// Invalidates the mapped memory range of the buffer to make it visible to the host
VkResult Buffer::invalidate(const MappedRange &range) {
    VkMappedMemoryRange mapped_range{};
    mapped_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mapped_range.memory = memory;
    mapped_range.offset = range.offset;
    mapped_range.size = range.size;
    return vkInvalidateMappedMemoryRanges(device.logical_device, 1, &mapped_range);
}

/// Retrieves the descriptor info
VkDescriptorBufferInfo Buffer::descriptor_info(const MappedRange &range) {
    return VkDescriptorBufferInfo{ .buffer = buffer, .offset = range.offset, .range = range.size };
}


/// Writes to the specified index
void Buffer::write(void *data, s32 index) {
    write(data, { instance_size, index * alignment_size });
}

/// Flushes the buffer at the specified index
VkResult Buffer::flush(s32 index) {
    return flush({ alignment_size, index * alignment_size });
}

/// Retrieves the descriptor info at the specified index
VkDescriptorBufferInfo Buffer::descriptor_info(int index) {
    return descriptor_info({ alignment_size, index * alignment_size });
}

/// Invalidates the buffer at the specified index
VkResult Buffer::invalidate(s32 index) {
    return invalidate({ alignment_size, index * alignment_size });
}

/// Returns the minimum instance size required to be cmpatible with devices min_offset_alignment
VkDeviceSize Buffer::alignment(VkDeviceSize instance_size, VkDeviceSize min_offset_alignment) {
    if (min_offset_alignment > 0) {
        return (instance_size + min_offset_alignment - 1) & ~(min_offset_alignment - 1);
    }
    return instance_size;
}

}// namespace rt
