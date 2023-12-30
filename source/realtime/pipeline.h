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

#ifndef REALTIME_PIPELINE_H
#define REALTIME_PIPELINE_H

#include <filesystem>

#include "device.h"

namespace rt {

    struct PipelineDescription {
        VkViewport viewport;
        VkRect2D scissor;
        VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
        VkPipelineRasterizationStateCreateInfo rasterization_info;
        VkPipelineMultisampleStateCreateInfo multisample_info;
        VkPipelineColorBlendAttachmentState color_blend_attachment;
        VkPipelineColorBlendStateCreateInfo color_blend_info;
        VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
        VkPipelineLayout pipeline_layout;
        VkRenderPass render_pass;
        u32 subpass;

        /// Creates a default pipeline description
        /// @param width The width of the viewport
        /// @param height The height of the viewport
        /// @return A pipeline description
        static PipelineDescription default_description(u32 width, u32 height);
    };

    class Pipeline {
    public:
        /// Creates a new pipeline instance from the specified vertex and fragment shaders
        /// @param device The device
        /// @param vertex The filesystem path to the SPIRV vertex shader
        /// @param fragment The filesystem path to the SPIRV fragment shader
        /// @param description The pipeline description
        Pipeline(Device &device,
                 const std::filesystem::path &vertex,
                 const std::filesystem::path &fragment,
                 const PipelineDescription &description);

        /// Destroys all pipeline objects
        ~Pipeline();

        /// A pipeline cannot be copied or moved
        Pipeline(const Pipeline &p) = delete;
        Pipeline &operator=(const Pipeline &p) = delete;
        Pipeline(Pipeline &&p) = delete;
        Pipeline &operator=(Pipeline &&p) = delete;

        /// Bind the pipeline for the specified command buffer
        /// @param command_buffer The recording command buffer
        void bind(VkCommandBuffer command_buffer) const;

    private:
        /// Creates or recreates a new Vulkan pipeline instance from the specified vertex and fragment shaders
        /// @param vertex The filesystem path to the SPIRV vertex shader
        /// @param fragment The filesystem path to the SPIRV fragment shader
        /// @param description The pipeline description
        void create_pipeline(const std::filesystem::path &vertex,
                             const std::filesystem::path &fragment,
                             const PipelineDescription &description);

        /// Creates a Vulkan shader module from the specified SPIR-V code
        /// @param code The SPIR-V shader code
        /// @param shader_module The shader module
        void create_shader_module(std::string_view code, VkShaderModule *shader_module) const;

        Device &device;
        VkPipeline graphics_pipeline;
        VkShaderModule vertex_shader_module;
        VkShaderModule fragment_shader_module;
    };

}// namespace rt

#endif// REALTIME_PIPELINE_H
