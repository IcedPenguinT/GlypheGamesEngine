#pragma once

#include "VulkanTypes.inl"

b8 CreateShaderModule(
    VulkanContext* context,
    const char* name,
    const char* typeStr,
    VkShaderStageFlagBits shaderStageFlag,
    u32 stageIndex,
    VulkanShaderStage* shaderStages
);