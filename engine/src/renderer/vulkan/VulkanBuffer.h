#pragma once

#include "VulkanTypes.inl"

b8 VulkanBufferCreate(
    VulkanContext* context,
    u64 size,
    VkBufferUsageFlagBits usage,
    u32 memoryPropertyFlag,
    b8 bindOnCreate,
    VulkanBuffer* outBuffer);

void VulkanBufferDestroy(VulkanContext* context, VulkanBuffer* buffer);

b8 VulkanBufferResize(
    VulkanContext* context,
    u64 newSize,
    VulkanBuffer* buffer,
    VkQueue queue,
    VkCommandPool pool);

void VulkanBufferBind(VulkanContext* context, VulkanBuffer* buffer, u64 offset);

void* VulkanBufferLockMemory(VulkanContext* context, VulkanBuffer* buffer, u64 offset, u64 size, u32 flags);
void VulkanBufferUnlockMemory(VulkanContext* context, VulkanBuffer* buffer);

void VulkanBufferLoadData(VulkanContext* context, VulkanBuffer* buffer, u64 offset, u64 size, u32 flags, const void* data);

void VulkanBufferCopyTo(
    VulkanContext* context,
    VkCommandPool pool,
    VkFence fence,
    VkQueue queue,
    VkBuffer source,
    u64 sourceOffset,
    VkBuffer dest,
    u64 destOffset,
    u64 size);