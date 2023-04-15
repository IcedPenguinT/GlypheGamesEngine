#pragma once

#include "renderer/vulkan/VulkanTypes.inl"
#include "renderer/RendererTypes.inl"

b8 VulkanObjectShaderCreate(VulkanContext* context, VulkanObjectShader* outShader);

void VulkanObjectShaderDestroy(VulkanContext* context, struct VulkanObjectShader* shader);

void VulkanObjectShaderUse(VulkanContext* context, struct VulkanObjectShader* shader);