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

#include "pipeline.h"
#include "utility.h"

#include <string_view>

namespace rt {

    PipelineDescription::PipelineDescription() { }

    /// Creates a new pipeline instance from the specified vertex and fragment shaders
    Pipeline::Pipeline(Device &device,
                       const std::filesystem::path &vertex,
                       const std::filesystem::path &fragment,
                       PipelineDescription description)
        : device{ device },
          graphics_pipeline{},
          vertex_shader_module{},
          fragment_shader_module{} {
        create_pipeline(vertex, fragment, description);
    }

    /// Creates or recreates a new Vulkan pipeline instance from the specified vertex and fragment shaders
    void Pipeline::create_pipeline(const std::filesystem::path &vertex,
                                   const std::filesystem::path &fragment,
                                   [[maybe_unused]] PipelineDescription description) {
        constexpr auto read_flags = std::ios::binary;
        auto vertex_code = read_file(vertex, read_flags).value_or("<invalid>");
        auto fragment_code = read_file(fragment, read_flags).value_or("<invalid>");

        printf("[pipeline] vertex shader code size: %d bytes.\n", static_cast<int>(vertex_code.size()));
        printf("[pipeline] fragment shader code size: %d bytes.\n", static_cast<int>(fragment_code.size()));

        create_shader_module(vertex_code, &vertex_shader_module);
        create_shader_module(fragment_code, &fragment_shader_module);
    }

    void Pipeline::create_shader_module(std::string_view code, VkShaderModule *shader_module) const {
        VkShaderModuleCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = code.size();
        create_info.pCode = reinterpret_cast<const u32 *>(code.data());

        if (vkCreateShaderModule(device.logical_device, &create_info, nullptr, shader_module) != VK_SUCCESS) {
            error(64, "[pipeline] Failed to create shader module!");
        }
    }

}// namespace rt
