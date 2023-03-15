#include "VulkanFramebuffer.h"

#include "core/Memory.h"

void VulkanFramebufferCreate(
    VulkanContext* context,
    VulkanRenderpass* renderpass,
    u32 width,
    u32 height,
    u32 attachmentCount,
    VkImageView* attachments,
    VulkanFramebuffer* outFramebuffer) {
    // Take a copy of the attachments, renderpass and attachment count
    outFramebuffer->attachments = Allocate(sizeof(VkImageView) * attachmentCount, MEMORY_TAG_RENDERER);
    for (u32 i = 0; i < attachmentCount; ++i) {
        outFramebuffer->attachments[i] = attachments[i];
    }
    outFramebuffer->renderpass = renderpass;
    outFramebuffer->attachmentCount = attachmentCount;

    // Creation info
    VkFramebufferCreateInfo framebufferCreateInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    framebufferCreateInfo.renderPass = renderpass->handle;
    framebufferCreateInfo.attachmentCount = attachmentCount;
    framebufferCreateInfo.pAttachments = outFramebuffer->attachments;
    framebufferCreateInfo.width = width;
    framebufferCreateInfo.height = height;
    framebufferCreateInfo.layers = 1;

    VK_CHECK(vkCreateFramebuffer(
        context->device.logicalDevice,
        &framebufferCreateInfo,
        context->allocator,
        &outFramebuffer->handle));
}

void VulkanFramebufferDestroy(VulkanContext* context, VulkanFramebuffer* framebuffer) {
    vkDestroyFramebuffer(context->device.logicalDevice, framebuffer->handle, context->allocator);
    if (framebuffer->attachments) {
        Free(framebuffer->attachments, sizeof(VkImageView) * framebuffer->attachmentCount, MEMORY_TAG_RENDERER);
        framebuffer->attachments = 0;
    }
    framebuffer->handle = 0;
    framebuffer->attachmentCount = 0;
    framebuffer->renderpass = 0;
}