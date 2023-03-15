#pragma once

#include "VulkanTypes.inl"

void VulkanSwapchainCreate(
    VulkanContext* context,
    u32 width,
    u32 height,
    VulkanSwapchain* outSwapchain);

void VulkanSwapchainRecreate(
    VulkanContext* context,
    u32 width,
    u32 height,
    VulkanSwapchain* swapchain);

void VulkanSwapchainDestroy(
    VulkanContext* context,
    VulkanSwapchain* swapchain);

b8 VulkanSwapchainAcquireNextImageIndex(
    VulkanContext* context,
    VulkanSwapchain* swapchain,
    u64 timeoutNs,
    VkSemaphore imageAvailableSemaphore,
    VkFence fence,
    u32* outImageIndex);

void VulkanSwapchainPresent(
    VulkanContext* context,
    VulkanSwapchain* swapchain,
    VkQueue graphicsQueue,
    VkQueue presentQueue,
    VkSemaphore renderCompleteSemaphore,
    u32 presentImageIndex);