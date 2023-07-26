#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <vector>

#include "mesh.h"
#include "utils.h"

class GraphicsPipeline {
   public:
    VkDevice device;
    VkPipelineLayout layout;
    VkPipeline pipeline;

    void destroy();
};

class GraphicsPipelineBuilder {
   public:
    GraphicsPipelineBuilder(VkDevice device) : device(device) {}
    GraphicsPipelineBuilder* add_shader(VkShaderStageFlagBits stage,
                                        const uint32_t buffer[], size_t size);
    GraphicsPipelineBuilder* set_extent(VkExtent2D extent);
    GraphicsPipelineBuilder* set_render_pass(VkRenderPass render_pass);
    GraphicsPipelineBuilder* add_push_constant_range(VkPushConstantRange range);
    void build(GraphicsPipeline* destination);

   private:
    VkRect2D scissor{};
    VkRenderPass render_pass{};
    VkDevice device{};

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages{};
    std::vector<VkPushConstantRange> push_constant_ranges{};

    VkViewport viewport{.minDepth = 0.f, .maxDepth = 1.f};

    VkPipelineVertexInputStateCreateInfo vertexinput_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    };
    VkPipelineInputAssemblyStateCreateInfo inputassembly_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    };
    VkPipelineRasterizationStateCreateInfo rasterization_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .lineWidth = 1.0f,
    };
    VkPipelineMultisampleStateCreateInfo multisample_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .minSampleShading = 1.0f,
    };
    VkPipelineViewportStateCreateInfo viewport_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };
    VkPipelineColorBlendAttachmentState colorblend_attachment{
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
    VkPipelineColorBlendStateCreateInfo colorblend_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOp =
            VK_LOGIC_OP_COPY,  // probably not used as .logicOpEnable is falsy
        .attachmentCount = 1,
        .pAttachments = &colorblend_attachment,
    };
    VkPipelineLayoutCreateInfo layout_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    };
    VkGraphicsPipelineCreateInfo pipeline_info{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pVertexInputState = &vertexinput_info,
        .pInputAssemblyState = &inputassembly_info,
        .pViewportState = &viewport_info,
        .pRasterizationState = &rasterization_info,
        .pMultisampleState = &multisample_info,
        .pColorBlendState = &colorblend_info,
    };
};
