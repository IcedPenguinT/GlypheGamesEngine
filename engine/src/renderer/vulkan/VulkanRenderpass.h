#pragma once

#include "VulkanTypes.inl"

void VulkanRenderpassCreate(
    VulkanContext* context, 
    VulkanRenderpass* outRenderpass,
    f32 x, f32 y, f32 w, f32 h,
    f32 r, f32 g, f32 b, f32 a,
    f32 depth,
    u32 stencil);

void VulkanRenderpassDestroy(VulkanContext* context, VulkanRenderpass* renderpass);

void VulkanRenderpassBegin(
    VulkanCommandBuffer* commandBuffer, 
    VulkanRenderpass* renderpass,
    VkFramebuffer frameBuffer);

void VulkanRenderpassEnd(VulkanCommandBuffer* commandBuffer, VulkanRenderpass* renderpass);