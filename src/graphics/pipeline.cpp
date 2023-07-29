#include "pipeline.h"

#include <vulkan/vulkan.h>

#include <cstdio>

GraphicsPipelineBuilder* GraphicsPipelineBuilder::add_shader(
    VkShaderStageFlagBits stage, const uint32_t buffer[], size_t size) {
    VkShaderModule module;
    VkShaderModuleCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size,
        .pCode = buffer,
    };
    vk_check(vkCreateShaderModule(device, &createInfo, nullptr, &module));
    VkPipelineShaderStageCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = stage,
        .module = module,
        .pName = "main",
    };
    shader_stages.push_back(info);
    return this;
}

GraphicsPipelineBuilder* GraphicsPipelineBuilder::set_extent(
    VkExtent2D extent) {
    viewport.width = extent.width;
    viewport.height = extent.height;
    scissor.extent = extent;
    return this;
}

GraphicsPipelineBuilder* GraphicsPipelineBuilder::set_render_pass(
    VkRenderPass render_pass) {
    this->render_pass = render_pass;
    return this;
}

GraphicsPipelineBuilder* GraphicsPipelineBuilder::add_push_constant_range(
    VkPushConstantRange range) {
    push_constant_ranges.push_back(range);
    return this;
}

GraphicsPipeline GraphicsPipelineBuilder::build() {
    GraphicsPipeline destination{};
    destination.device = device;

    auto bindings = Vertex::get_bindings();
    auto attributes = Vertex::get_attributes();

    vertexinput_info.vertexBindingDescriptionCount = bindings.size();
    vertexinput_info.pVertexBindingDescriptions = bindings.data();
    vertexinput_info.vertexAttributeDescriptionCount = attributes.size();
    vertexinput_info.pVertexAttributeDescriptions = attributes.data();

    layout_info.pushConstantRangeCount = push_constant_ranges.size();
    layout_info.pPushConstantRanges = push_constant_ranges.data();

    vk_check(vkCreatePipelineLayout(device, &layout_info, nullptr,
                                    &destination.layout));

    pipeline_info.stageCount = shader_stages.size();
    pipeline_info.pStages = shader_stages.data();
    pipeline_info.layout = destination.layout;
    pipeline_info.renderPass = render_pass;
    vk_check(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
                                       &pipeline_info, nullptr,
                                       &destination.pipeline));

    for (auto stage : shader_stages) {
        vkDestroyShaderModule(device, stage.module, nullptr);
    }

    return destination;
}

void GraphicsPipeline::destroy() {
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyPipelineLayout(device, layout, nullptr);
}
