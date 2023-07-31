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
    GraphicsPipelineBuilder* add_shader(VkShaderStageFlagBits stage,
                                        const uint32_t buffer[], size_t size);
    GraphicsPipelineBuilder* set_extent(VkExtent2D extent);
    GraphicsPipelineBuilder* set_render_pass(VkRenderPass render_pass);
    GraphicsPipelineBuilder* add_push_constant_range(VkPushConstantRange range);
    GraphicsPipeline build();

    GraphicsPipelineBuilder(VkDevice device)
        : device(device),
          scissor(),
          render_pass(),
          shader_stages(),
          push_constant_ranges(),

          viewport(VkViewport{.maxDepth = 1.f}),

          vertexinput_info(VkPipelineVertexInputStateCreateInfo{
              .sType =
                  VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO}),

          inputassembly_info(VkPipelineInputAssemblyStateCreateInfo{
              .sType =
                  VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
              .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST}),

          rasterization_info(VkPipelineRasterizationStateCreateInfo{
              .sType =
                  VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
              .polygonMode = VK_POLYGON_MODE_FILL,
              .frontFace = VK_FRONT_FACE_CLOCKWISE,
              .lineWidth = 1.0f}),

          multisample_info(VkPipelineMultisampleStateCreateInfo{
              .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
              .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
              .minSampleShading = 1.0f}),

          viewport_info(VkPipelineViewportStateCreateInfo{
              .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
              .viewportCount = 1,
              .pViewports = &viewport,
              .scissorCount = 1,
              .pScissors = &scissor}),

          colorblend_attachment(VkPipelineColorBlendAttachmentState{
              .colorWriteMask =
                  VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT}),

          colorblend_info(VkPipelineColorBlendStateCreateInfo{
              .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
              // probably not used as .logicOpEnable is falsy
              .logicOp = VK_LOGIC_OP_COPY,
              .attachmentCount = 1,
              .pAttachments = &colorblend_attachment}),

          depthstencil_info(VkPipelineDepthStencilStateCreateInfo{
              .sType =
                  VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
              .depthTestEnable = VK_TRUE,
              .depthWriteEnable = VK_TRUE,
              .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
              .depthBoundsTestEnable = VK_FALSE,
              .stencilTestEnable = VK_FALSE,
              .minDepthBounds = 0.0f,
              .maxDepthBounds = 1.0f,
          }),

          layout_info(VkPipelineLayoutCreateInfo{
              .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO}),

          pipeline_info(VkGraphicsPipelineCreateInfo{
              .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
              .pVertexInputState = &vertexinput_info,
              .pInputAssemblyState = &inputassembly_info,
              .pViewportState = &viewport_info,
              .pRasterizationState = &rasterization_info,
              .pMultisampleState = &multisample_info,
              .pDepthStencilState = &depthstencil_info,
              .pColorBlendState = &colorblend_info}) {}

   private:
    VkRect2D scissor;
    VkRenderPass render_pass;
    VkDevice device;

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    std::vector<VkPushConstantRange> push_constant_ranges;

    VkViewport viewport;

    VkPipelineInputAssemblyStateCreateInfo inputassembly_info;
    VkPipelineVertexInputStateCreateInfo vertexinput_info;
    VkPipelineRasterizationStateCreateInfo rasterization_info;
    VkPipelineMultisampleStateCreateInfo multisample_info;
    VkPipelineViewportStateCreateInfo viewport_info;
    VkPipelineColorBlendAttachmentState colorblend_attachment;
    VkPipelineColorBlendStateCreateInfo colorblend_info;
    VkPipelineDepthStencilStateCreateInfo depthstencil_info;
    VkPipelineLayoutCreateInfo layout_info;
    VkGraphicsPipelineCreateInfo pipeline_info;
};
