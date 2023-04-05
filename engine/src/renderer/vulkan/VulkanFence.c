#include "VulkanFence.h"

#include "core/Logger.h"

void VulkanFenceCreate(
    VulkanContext* context,
    b8 createSignaled,
    VulkanFence* outFence) {

    // Make sure to signal the fence if required.
    outFence->isSignaled = createSignaled;
    VkFenceCreateInfo fenceCreateInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    if (outFence->isSignaled) {
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }

    VK_CHECK(vkCreateFence(
        context->device.logicalDevice,
        &fenceCreateInfo,
        context->allocator,
        &outFence->handle));
}

void VulkanFenceDestroy(VulkanContext* context, VulkanFence* fence) {
    if (fence->handle) {
        vkDestroyFence(
            context->device.logicalDevice,
            fence->handle,
            context->allocator);
        fence->handle = 0;
    }
    fence->isSignaled = false;
}

b8 VulkanFenceWait(VulkanContext* context, VulkanFence* fence, u64 timeoutNs) {
    if (!fence->isSignaled) {
        VkResult result = vkWaitForFences(
            context->device.logicalDevice,
            1,
            &fence->handle,
            true,
            timeoutNs);
        switch (result) {
            case VK_SUCCESS:
                fence->isSignaled = true;
                return true;
            case VK_TIMEOUT:
                KWARNING("vk_fence_wait - Timed out");
                break;
            case VK_ERROR_DEVICE_LOST:
                KERROR("vk_fence_wait - VK_ERROR_DEVICE_LOST.");
                break;
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                KERROR("vk_fence_wait - VK_ERROR_OUT_OF_HOST_MEMORY.");
                break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                KERROR("vk_fence_wait - VK_ERROR_OUT_OF_DEVICE_MEMORY.");
                break;
            default:
                KERROR("vk_fence_wait - An unknown error has occurred.");
                break;
        }
    } else {
        // If already signaled, do not wait.
        return true;
    }

    return false;
}

void VulkanFenceReset(VulkanContext* context, VulkanFence* fence) {
    if (fence->isSignaled) {
        VK_CHECK(vkResetFences(context->device.logicalDevice, 1, &fence->handle));
        fence->isSignaled = false;
    }
}