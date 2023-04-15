#include "VulkanObjectShader.h"

#include "core/Logger.h"
#include "core/Memory.h"
#include "math/MathTypes.h"

#include "renderer/vulkan/VulkanShaderUtils.h"
#include "renderer/vulkan/VulkanPipeline.h"

#define BUILTIN_SHADER_NAME_OBJECT "Builtin.ObjectShader"


b8 VulkanObjectShaderCreate(VulkanContext* context, VulkanObjectShader* outShader) {
    // Shader module init per stage.
    char stageTypeStrs[OBJECT_SHADER_STAGE_COUNT][5] = {"vert", "frag"};
    VkShaderStageFlagBits stageTypes[OBJECT_SHADER_STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
        if (!CreateShaderModule(context, BUILTIN_SHADER_NAME_OBJECT, stageTypeStrs[i], stageTypes[i], i, outShader->stages)) {
            KERROR("Unable to create %s shader module for '%s'.", stageTypeStrs[i], BUILTIN_SHADER_NAME_OBJECT);
            return false;
        }
    }

    // TODO: Descriptors

    // Pipeline creation
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (f32)context->framebufferHeight;
    viewport.width = (f32)context->framebufferWidth;
    viewport.height = -(f32)context->framebufferHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Scissor
    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = context->framebufferWidth;
    scissor.extent.height = context->framebufferHeight;

    // Attributes
    u32 offset = 0;
    const i32 attributeCount = 1;
    VkVertexInputAttributeDescription attributeDescriptions[attributeCount];
    // Position
    VkFormat formats[attributeCount] = {
        VK_FORMAT_R32G32B32_SFLOAT
    };
    u64 sizes[attributeCount] = {
        sizeof(Vector3)
    };
    for (u32 i = 0; i < attributeCount; ++i) {
        attributeDescriptions[i].binding = 0;   // binding index - should match binding desc
        attributeDescriptions[i].location = i;  // attrib location
        attributeDescriptions[i].format = formats[i];
        attributeDescriptions[i].offset = offset;
        offset += sizes[i];
    }

    // TODO: Desciptor set layouts.

    // Stages
    // NOTE: Should match the number of shader->stages.
    VkPipelineShaderStageCreateInfo stageCreateInfo[OBJECT_SHADER_STAGE_COUNT];
    ZeroMemory(stageCreateInfo, sizeof(stageCreateInfo));
    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
        stageCreateInfo[i].sType = outShader->stages[i].shaderStageCreateInfo.sType;
        stageCreateInfo[i] = outShader->stages[i].shaderStageCreateInfo;
    }

    if (!VulkanGraphicsPipelineCreate(
            context,
            &context->mainRenderpass,
            attributeCount,
            attributeDescriptions,
            0,
            0,
            OBJECT_SHADER_STAGE_COUNT,
            stageCreateInfo,
            viewport,
            scissor,
            false,
            &outShader->pipeline)) {
        KERROR("Failed to load graphics pipeline for object shader.");
        return false;
    }

    return true;
}

void VulkanObjectShaderDestroy(VulkanContext* context, struct VulkanObjectShader* shader) {
    VulkanPipelineDestroy(context, &shader->pipeline);

    // Destroy shader modules.
    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
        vkDestroyShaderModule(context->device.logicalDevice, shader->stages[i].handle, context->allocator);
        shader->stages[i].handle = 0;
    }
}

void VulkanObjectShaderUse(VulkanContext* context, struct VulkanObjectShader* shader) {
    u32 imageIndex = context->imageIndex;
    VulkanPipelineBind(&context->graphicsCommandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, &shader->pipeline);
}