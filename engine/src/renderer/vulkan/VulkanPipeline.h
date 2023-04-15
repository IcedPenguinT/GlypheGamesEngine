#pragma once

#include "VulkanTypes.inl"

b8 VulkanGraphicsPipelineCreate(
    VulkanContext* context,
    VulkanRenderpass* renderpass,
    u32 attributeCount,
    VkVertexInputAttributeDescription* attributes,
    u32 descriptorSetLayoutCount,
    VkDescriptorSetLayout* descriptorSetLayouts,
    u32 stageCount,
    VkPipelineShaderStageCreateInfo* stages,
    VkViewport viewport,
    VkRect2D scissor,
    b8 isWireframe,
    VulkanPipeline* outPipeline);

void VulkanPipelineDestroy(VulkanContext* context, VulkanPipeline* pipeline);

void VulkanPipelineBind(VulkanCommandBuffer* commandBuffer, VkPipelineBindPoint bindPoint, VulkanPipeline* pipeline);