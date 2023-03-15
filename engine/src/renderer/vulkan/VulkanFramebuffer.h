#pragma once

#include "VulkanTypes.inl"

void VulkanFramebufferCreate(
    VulkanContext* context,
    VulkanRenderpass* renderpass,
    u32 width,
    u32 height,
    u32 attachmentCount,
    VkImageView* attachments,
    VulkanFramebuffer* outFramebuffer);

void VulkanFramebufferDestroy(VulkanContext* context, VulkanFramebuffer* framebuffer);