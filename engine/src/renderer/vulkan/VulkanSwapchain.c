#include "VulkanSwapchain.h"

#include "core/Logger.h"
#include "core/Memory.h"
#include "VulkanDevice.h"
#include "VulkanImage.h"

void Create(VulkanContext* context, u32 width, u32 height, VulkanSwapchain* swapchain);
void Destory(VulkanContext* context, VulkanSwapchain* swapchain);

void VulkanSwapchainCreate(
    VulkanContext* context,
    u32 width,
    u32 height,
    VulkanSwapchain* outSwapchain) {
    // Simply Create a new one.
    Create(context, width, height, outSwapchain);
}

void VulkanSwapchainRecreate(
    VulkanContext* context,
    u32 width,
    u32 height,
    VulkanSwapchain* swapchain) {
    // Destroy the old and Create a new one.
    Destory(context, swapchain);
    Create(context, width, height, swapchain);
}

void VulkanSwapchainDestroy(
    VulkanContext* context,
    VulkanSwapchain* swapchain) {
    Destory(context, swapchain);
}

b8 VulkanSwapchainAquireNextImageIndex(
    VulkanContext* context,
    VulkanSwapchain* swapchain,
    u64 timeoutNs,
    VkSemaphore imageAvailableSemaphore,
    VkFence fence,
    u32* outImageIndex) {
    VkResult result = vkAcquireNextImageKHR(
        context->device.logicalDevice,
        swapchain->handle,
        timeoutNs,
        imageAvailableSemaphore,
        fence,
        outImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // Trigger swapchain recreation, then boot out of the render loop.
        VulkanSwapchainRecreate(context, context->framebufferWidth, context->framebufferHeight, swapchain);
        return FALSE;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        KFATAL("Failed to acquire swapchain image!");
        return FALSE;
    }

    return TRUE;
}

void VulkanSwapchainPresent(
    VulkanContext* context,
    VulkanSwapchain* swapchain,
    VkQueue graphicsQueue,
    VkQueue presentQueue,
    VkSemaphore renderCompleteSemaphore,
    u32 presentImageIndex) {
    // Return the image to the swapchain for presentation.
    VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderCompleteSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain->handle;
    presentInfo.pImageIndices = &presentImageIndex;
    presentInfo.pResults = 0;

    VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        // Swapchain is out of date, suboptimal or a framebuffer resize has occurred. Trigger swapchain recreation.
        VulkanSwapchainRecreate(context, context->framebufferWidth, context->framebufferHeight, swapchain);
    } else if (result != VK_SUCCESS) {
        KFATAL("Failed to present swap chain image!");
    }
    // Increment (and loop) the index.
    context->currentFrame = (context->currentFrame + 1) % swapchain->maxFramesInFlight;
}

void Create(VulkanContext* context, u32 width, u32 height, VulkanSwapchain* swapchain) {
    VkExtent2D swapchainExtent = {width, height};
    swapchain->maxFramesInFlight = 2;

    // Choose a swap surface format.
    b8 found = FALSE;
    for (u32 i = 0; i < context->device.swapchainSupport.formatCount; ++i) {
        VkSurfaceFormatKHR format = context->device.swapchainSupport.formats[i];
        // Preferred formats
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            swapchain->imageFormat = format;
            found = TRUE;
            break;
        }
    }

    if (!found) {
        swapchain->imageFormat = context->device.swapchainSupport.formats[0];
    }


    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (u32 i = 0; i < context->device.swapchainSupport.presentModeCount; ++i) {
        VkPresentModeKHR mode = context->device.swapchainSupport.presentModes[i];
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            presentMode = mode;
            break;
        }
    }

    // Requery swapchain support.
    VulkanDeviceQuerySwapchainSupport(
        context->device.physicalDevice,
        context->surface,
        &context->device.swapchainSupport);

    // Swapchain extent
    if (context->device.swapchainSupport.capabilities.currentExtent.width != UINT32_MAX) {
        swapchainExtent = context->device.swapchainSupport.capabilities.currentExtent;
    }

    // Clamp to the value allowed by the GPU.
    VkExtent2D min = context->device.swapchainSupport.capabilities.minImageExtent;
    VkExtent2D max = context->device.swapchainSupport.capabilities.maxImageExtent;
    swapchainExtent.width = KCLAMP(swapchainExtent.width, min.width, max.width);
    swapchainExtent.height = KCLAMP(swapchainExtent.height, min.height, max.height);

    u32 imageCount = context->device.swapchainSupport.capabilities.minImageCount + 1;
    if (context->device.swapchainSupport.capabilities.maxImageCount > 0 && imageCount > context->device.swapchainSupport.capabilities.maxImageCount) {
        imageCount = context->device.swapchainSupport.capabilities.maxImageCount;
    }

    // Swapchain Create info
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    swapchainCreateInfo.surface = context->surface;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = swapchain->imageFormat.format;
    swapchainCreateInfo.imageColorSpace = swapchain->imageFormat.colorSpace;
    swapchainCreateInfo.imageExtent = swapchainExtent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Setup the queue family indices
    if (context->device.graphicsQueueIndex != context->device.presentQueueIndex) {
        u32 queueFamilyIndices[] = {
            (u32)context->device.graphicsQueueIndex,
            (u32)context->device.presentQueueIndex};
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0;
        swapchainCreateInfo.pQueueFamilyIndices = 0;
    }

    swapchainCreateInfo.preTransform = context->device.swapchainSupport.capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = 0;

    VK_CHECK(vkCreateSwapchainKHR(context->device.logicalDevice, &swapchainCreateInfo, context->allocator, &swapchain->handle));

    // Start with a zero frame index.
    context->currentFrame = 0;

    // Images
    swapchain->imageCount = 0;
    VK_CHECK(vkGetSwapchainImagesKHR(context->device.logicalDevice, swapchain->handle, &swapchain->imageCount, 0));
    if (!swapchain->images) {
        swapchain->images = (VkImage*)Allocate(sizeof(VkImage) * swapchain->imageCount, MEMORY_TAG_RENDERER);
    }
    if (!swapchain->views) {
        swapchain->views = (VkImageView*)Allocate(sizeof(VkImageView) * swapchain->imageCount, MEMORY_TAG_RENDERER);
    }
    VK_CHECK(vkGetSwapchainImagesKHR(context->device.logicalDevice, swapchain->handle, &swapchain->imageCount, swapchain->images));

    // Views
    for (u32 i = 0; i < swapchain->imageCount; ++i) {
        VkImageViewCreateInfo viewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        viewInfo.image = swapchain->images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = swapchain->imageFormat.format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VK_CHECK(vkCreateImageView(context->device.logicalDevice, &viewInfo, context->allocator, &swapchain->views[i]));
    }

    // Depth resources
    if (!VulkanDeviceDetectDepthFormat(&context->device)) {
        context->device.depthFormat = VK_FORMAT_UNDEFINED;
        KFATAL("Failed to find a supported format!");
    }

    // Create depth image and its view.
    VulkanImageCreate(
        context,
        VK_IMAGE_TYPE_2D,
        swapchainExtent.width,
        swapchainExtent.height,
        context->device.depthFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        TRUE,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        &swapchain->depthAttachment);

    KINFO("Swapchain created successfully.");
}

void Destory(VulkanContext* context, VulkanSwapchain* swapchain) {
    VulkanImageDestroy(context, &swapchain->depthAttachment);

    // Only Destory the views, not the images, since those are owned by the swapchain and are thus
    // destroyed when it is.
    for (u32 i = 0; i < swapchain->imageCount; ++i) {
        vkDestroyImageView(context->device.logicalDevice, swapchain->views[i], context->allocator);
    }

    vkDestroySwapchainKHR(context->device.logicalDevice, swapchain->handle, context->allocator);
}