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


#include <array>

#include "utility.h"

namespace rt {

    /// Creates a default pipeline description
    PipelineDescription PipelineDescription::default_description(u32 width, u32 height) {
        PipelineDescription result{};

        /// TODO(elias): Add support for specifying different topologies
        result.input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        result.input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        result.input_assembly_info.primitiveRestartEnable = VK_FALSE;

        result.viewport.x = 0.0f;
        result.viewport.y = 0.0f;
        result.viewport.width = static_cast<f32>(width);
        result.viewport.height = static_cast<f32>(height);
        result.viewport.minDepth = 0.0f;
        result.viewport.maxDepth = 1.0f;

        result.scissor.offset = { 0, 0 };
        result.scissor.extent = { width, height };

        result.rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        result.rasterization_info.depthClampEnable = VK_FALSE;
        result.rasterization_info.rasterizerDiscardEnable = VK_FALSE;
        result.rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
        result.rasterization_info.lineWidth = 1.0f;

        /// TODO(elias): Try out back culling
        result.rasterization_info.cullMode = VK_CULL_MODE_NONE;
        result.rasterization_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
        result.rasterization_info.depthBiasEnable = VK_FALSE;
        result.rasterization_info.depthBiasConstantFactor = 0.0f;
        result.rasterization_info.depthBiasClamp = 0.0f;
        result.rasterization_info.depthBiasSlopeFactor = 0.0f;

        result.multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        result.multisample_info.sampleShadingEnable = VK_FALSE;
        result.multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        result.multisample_info.minSampleShading = 1.0f;
        result.multisample_info.pSampleMask = nullptr;
        result.multisample_info.alphaToCoverageEnable = VK_FALSE;
        result.multisample_info.alphaToOneEnable = VK_FALSE;

        result.color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                       VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        result.color_blend_attachment.blendEnable = VK_FALSE;
        result.color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        result.color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        result.color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
        result.color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        result.color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        result.color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

        result.color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        result.color_blend_info.logicOpEnable = VK_FALSE;
        result.color_blend_info.logicOp = VK_LOGIC_OP_COPY;
        result.color_blend_info.attachmentCount = 1;
        result.color_blend_info.pAttachments = &result.color_blend_attachment;
        result.color_blend_info.blendConstants[0] = 0.0f;
        result.color_blend_info.blendConstants[1] = 0.0f;
        result.color_blend_info.blendConstants[2] = 0.0f;
        result.color_blend_info.blendConstants[3] = 0.0f;

        result.depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        result.depth_stencil_info.depthTestEnable = VK_TRUE;
        result.depth_stencil_info.depthWriteEnable = VK_TRUE;
        result.depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
        result.depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
        result.depth_stencil_info.minDepthBounds = 0.0f;
        result.depth_stencil_info.maxDepthBounds = 1.0f;
        result.depth_stencil_info.stencilTestEnable = VK_FALSE;
        result.depth_stencil_info.front = {};
        result.depth_stencil_info.back = {};

        result.pipeline_layout = nullptr;
        result.render_pass = nullptr;
        result.subpass = 0;

        return result;
    }

    /// Creates a new pipeline instance from the specified vertex and fragment shaders
    Pipeline::Pipeline(Device &device,
                       const std::filesystem::path &vertex,
                       const std::filesystem::path &fragment,
                       const PipelineDescription &description)
        : device{ device },
          graphics_pipeline{},
          vertex_shader_module{},
          fragment_shader_module{} {
        create_pipeline(vertex, fragment, description);
    }

    /// Destroys all pipeline objects
    Pipeline::~Pipeline() {
        vkDestroyShaderModule(device.logical_device, vertex_shader_module, nullptr);
        vkDestroyShaderModule(device.logical_device, fragment_shader_module, nullptr);
        vkDestroyPipeline(device.logical_device, graphics_pipeline, nullptr);
    }

    /// Bind the pipeline for the specified command buffer
    void Pipeline::bind(VkCommandBuffer command_buffer) const {
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);
    }

    /// Creates or recreates a new Vulkan pipeline instance from the specified vertex and fragment shaders
    void Pipeline::create_pipeline(const std::filesystem::path &vertex,
                                   const std::filesystem::path &fragment,
                                   const PipelineDescription &description) {
        assert(description.pipeline_layout != VK_NULL_HANDLE &&
               "[pipeline]  Cannot create graphics pipeline: no pipeline_layout provided in pipeline description!");
        assert(description.render_pass != VK_NULL_HANDLE &&
               "[pipeline] Cannot create graphics pipeline: no render_pass provided in description!");

        constexpr auto read_flags = std::ios::binary;
        constexpr auto invalid = "<<>>";
        auto vertex_code = read_file(vertex, read_flags).value_or(invalid);
        auto fragment_code = read_file(fragment, read_flags).value_or(invalid);

        create_shader_module(vertex_code, &vertex_shader_module);
        create_shader_module(fragment_code, &fragment_shader_module);

        std::array<VkPipelineShaderStageCreateInfo, 2> stages = {};
        stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        stages[0].module = vertex_shader_module;
        stages[0].pName = "main";
        stages[0].flags = 0;
        stages[0].pNext = nullptr;
        stages[0].pSpecializationInfo = nullptr;
        stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stages[1].module = fragment_shader_module;
        stages[1].pName = "main";
        stages[1].flags = 0;
        stages[1].pNext = nullptr;
        stages[1].pSpecializationInfo = nullptr;

        VkPipelineVertexInputStateCreateInfo vertex_input_info{};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.vertexAttributeDescriptionCount = 0;
        vertex_input_info.vertexBindingDescriptionCount = 0;
        vertex_input_info.pVertexAttributeDescriptions = nullptr;
        vertex_input_info.pVertexBindingDescriptions = nullptr;

        VkPipelineViewportStateCreateInfo viewport_info{};
        viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_info.viewportCount = 1;
        viewport_info.pViewports = &description.viewport;
        viewport_info.scissorCount = 1;
        viewport_info.pScissors = &description.scissor;

        VkGraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_info.stageCount = 2;
        pipeline_info.pStages = stages.data();
        pipeline_info.pVertexInputState = &vertex_input_info;
        pipeline_info.pInputAssemblyState = &description.input_assembly_info;
        pipeline_info.pViewportState = &viewport_info;
        pipeline_info.pRasterizationState = &description.rasterization_info;
        pipeline_info.pMultisampleState = &description.multisample_info;
        pipeline_info.pColorBlendState = &description.color_blend_info;
        pipeline_info.pDepthStencilState = &description.depth_stencil_info;
        pipeline_info.pDynamicState = nullptr;
        pipeline_info.layout = description.pipeline_layout;
        pipeline_info.renderPass = description.render_pass;
        pipeline_info.subpass = description.subpass;
        pipeline_info.basePipelineIndex = -1;
        pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(device.logical_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr,
                                      &graphics_pipeline) != VK_SUCCESS) {
            error(64, "[pipeline] Could not create graphics pipeline!");
        }
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
