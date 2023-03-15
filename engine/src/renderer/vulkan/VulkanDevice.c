#include "VulkanDevice.h"
#include "core/Logger.h"
#include "core/String.h"
#include "core/Memory.h"
#include "containers/Darray.h"

typedef struct vulkan_physical_device_requirements {
    b8 graphics;
    b8 present;
    b8 compute;
    b8 transfer;
    // darray
    const char** deviceExtensionNames;
    b8 samplerAnisotropy;
    b8 discreteGpu;
} vulkan_physical_device_requirements;

typedef struct VulkanPhysicalDeviceQueueFamilyInfo {
    u32 graphicsFamilyIndex;
    u32 presentFamilyIndex;
    u32 computeFamilyIndex;
    u32 transferFamilyIndex;
} VulkanPhysicalDeviceQueueFamilyInfo;

b8 SelectPhysicalDevice(VulkanContext* context);
b8 PhysicalDeviceMeetsRequirements(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties* properties,
    const VkPhysicalDeviceFeatures* features,
    const vulkan_physical_device_requirements* requirements,
    VulkanPhysicalDeviceQueueFamilyInfo* outQueueFamilyInfo,
    VulkanSwapchainSupportInfo* outSwapchainSupport);

b8 VulkanDeviceCreate(VulkanContext* context) {
    if (!SelectPhysicalDevice(context)) {
        return FALSE;
    }

     KINFO("Creating logical device...");
    // NOTE: Do not create additional queues for shared indices.
    b8 presentSharesGraphicsQueue = context->device.graphicsQueueIndex == context->device.presentQueueIndex;
    b8 transferSharesGraphicsQueue = context->device.graphicsQueueIndex == context->device.transferQueueIndex;
    u32 indexCount = 1;
    if (!presentSharesGraphicsQueue) {
        indexCount++;
    }
    if (!transferSharesGraphicsQueue) {
        indexCount++;
    }
    u32 indices[indexCount];
    u8 index = 0;
    indices[index++] = context->device.graphicsQueueIndex;
    if (!presentSharesGraphicsQueue) {
        indices[index++] = context->device.presentQueueIndex;
    }
    if (!transferSharesGraphicsQueue) {
        indices[index++] = context->device.transferQueueIndex;
    }

    VkDeviceQueueCreateInfo queueCreateInfos[indexCount];
    for (u32 i = 0; i < indexCount; ++i) {
        queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[i].queueFamilyIndex = indices[i];
        queueCreateInfos[i].queueCount = 1;
        //if (indices[i] == context->device.graphicsQueueIndex) {
        //    queueCreateInfos[i].queueCount = 2;
        //}
        queueCreateInfos[i].flags = 0;
        queueCreateInfos[i].pNext = 0;
        f32 queuePriority = 1.0f;
        queueCreateInfos[i].pQueuePriorities = &queuePriority;
    }

    // Request device features.
    // TODO: should be config driven
    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;  // Request anistrophy

    VkDeviceCreateInfo deviceCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    deviceCreateInfo.queueCreateInfoCount = indexCount;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = 1;
    const char* extensionNames = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    deviceCreateInfo.ppEnabledExtensionNames = &extensionNames;

    // Deprecated and ignored, so pass nothing.
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = 0;

    // Create the device.
    VK_CHECK(vkCreateDevice(
        context->device.physicalDevice,
        &deviceCreateInfo,
        context->allocator,
        &context->device.logicalDevice));

    KINFO("Logical device created.");

    // Get queues.
    vkGetDeviceQueue(
        context->device.logicalDevice,
        context->device.graphicsQueueIndex,
        0,
        &context->device.graphicsQueue);

    vkGetDeviceQueue(
        context->device.logicalDevice,
        context->device.presentQueueIndex,
        0,
        &context->device.presentQueue);

    vkGetDeviceQueue(
        context->device.logicalDevice,
        context->device.transferQueueIndex,
        0,
        &context->device.transferQueue);
    KINFO("Queues obtained.");


    return TRUE;
}

void VulkanDeviceDestory(VulkanContext* context) {
    context->device.graphicsQueue = 0;
    context->device.presentQueue = 0;
    context->device.transferQueue = 0;

    // Destroy logical device
    KINFO("Destroying logical device...");
    if (context->device.logicalDevice) {
        vkDestroyDevice(context->device.logicalDevice, context->allocator);
        context->device.logicalDevice = 0;
    }

    // Physical devices are not destroyed.
    KINFO("Releasing physical device resources...");
    context->device.physicalDevice = 0;

    if (context->device.swapchainSupport.formats) {
        Free(
            context->device.swapchainSupport.formats,
            sizeof(VkSurfaceFormatKHR) * context->device.swapchainSupport.formatCount,
            MEMORY_TAG_RENDERER);
        context->device.swapchainSupport.formats = 0;
        context->device.swapchainSupport.formatCount = 0;
    }

    if (context->device.swapchainSupport.presentModes) {
        Free(
            context->device.swapchainSupport.presentModes,
            sizeof(VkPresentModeKHR) * context->device.swapchainSupport.presentModeCount,
            MEMORY_TAG_RENDERER);
        context->device.swapchainSupport.presentModes = 0;
        context->device.swapchainSupport.presentModeCount = 0;
    }

    ZeroMemory(
        &context->device.swapchainSupport.capabilities,
        sizeof(context->device.swapchainSupport.capabilities));

    context->device.graphicsQueueIndex = -1;
    context->device.presentQueueIndex = -1;
    context->device.transferQueueIndex = -1;
}

void VulkanDeviceQuerySwapchainSupport(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    VulkanSwapchainSupportInfo* outSupportInfo) {
    // Surface capabilities
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physicalDevice,
        surface,
        &outSupportInfo->capabilities));

    // Surface formats
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice,
        surface,
        &outSupportInfo->formatCount,
        0));

    if (outSupportInfo->formatCount != 0) {
        if (!outSupportInfo->formats) {
            outSupportInfo->formats = Allocate(sizeof(VkSurfaceFormatKHR) * outSupportInfo->formatCount, MEMORY_TAG_RENDERER);
        }
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
            physicalDevice,
            surface,
            &outSupportInfo->formatCount,
            outSupportInfo->formats));
    }

    // Present modes
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice,
        surface,
        &outSupportInfo->presentModeCount,
        0));
    if (outSupportInfo->presentModeCount != 0) {
        if (!outSupportInfo->presentModes) {
            outSupportInfo->presentModes = Allocate(sizeof(VkPresentModeKHR) * outSupportInfo->presentModeCount, MEMORY_TAG_RENDERER);
        }
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice,
            surface,
            &outSupportInfo->presentModeCount,
            outSupportInfo->presentModes));
    }
}

b8 VulkanDeviceDetectDepthFormat(VulkanDevice* device) {
    // Format candidates
    const u64 candidateCount = 3;
    VkFormat candidates[3] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT};

    u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    for (u64 i = 0; i < candidateCount; ++i) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(device->physicalDevice, candidates[i], &properties);

        if ((properties.linearTilingFeatures & flags) == flags) {
            device->depthFormat = candidates[i];
            return TRUE;
        } else if ((properties.optimalTilingFeatures & flags) == flags) {
            device->depthFormat = candidates[i];
            return TRUE;
        }
    }

    return FALSE;
}

b8 SelectPhysicalDevice(VulkanContext* context) {
    u32 physicalDeviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physicalDeviceCount, 0));
    if (physicalDeviceCount == 0) {
        KFATAL("No devices which support Vulkan were found.");
        return FALSE;
    }

    VkPhysicalDevice physicalDevice[physicalDeviceCount];
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physicalDeviceCount, physicalDevice));
    for (u32 i = 0; i < physicalDeviceCount; ++i) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevice[i], &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(physicalDevice[i], &features);

        VkPhysicalDeviceMemoryProperties memory;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice[i], &memory);

        // TODO: These requirements should probably be driven by engine
        // configuration.
        vulkan_physical_device_requirements requirements = {};
        requirements.graphics = TRUE;
        requirements.present = TRUE;
        requirements.transfer = TRUE;
        // NOTE: Enable this if compute will be required.
        // requirements.compute = TRUE;
        requirements.samplerAnisotropy = TRUE;
        requirements.discreteGpu = TRUE;
        requirements.deviceExtensionNames = DarrayCreate(const char*);
        DarrayPush(requirements.deviceExtensionNames, &VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        VulkanPhysicalDeviceQueueFamilyInfo queueInfo = {};
        b8 result = PhysicalDeviceMeetsRequirements(
            physicalDevice[i],
            context->surface,
            &properties,
            &features,
            &requirements,
            &queueInfo,
            &context->device.swapchainSupport);

        if (result) {
            KINFO("Selected device: '%s'.", properties.deviceName);
            // GPU type, etc.
            switch (properties.deviceType) {
                default:
                case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                    KINFO("GPU type is Unknown.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    KINFO("GPU type is Integrated.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    KINFO("GPU type is Descrete.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    KINFO("GPU type is Virtual.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    KINFO("GPU type is CPU.");
                    break;
            }

            KINFO(
                "GPU Driver version: %d.%d.%d",
                VK_VERSION_MAJOR(properties.driverVersion),
                VK_VERSION_MINOR(properties.driverVersion),
                VK_VERSION_PATCH(properties.driverVersion));

            // Vulkan API version.
            KINFO(
                "Vulkan API version: %d.%d.%d",
                VK_VERSION_MAJOR(properties.apiVersion),
                VK_VERSION_MINOR(properties.apiVersion),
                VK_VERSION_PATCH(properties.apiVersion));

            // Memory information
            for (u32 j = 0; j < memory.memoryHeapCount; ++j) {
                f32 memorySizeGib = (((f32)memory.memoryHeaps[j].size) / 1024.0f / 1024.0f / 1024.0f);
                if (memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                    KINFO("Local GPU memory: %.2f GiB", memorySizeGib);
                } else {
                    KINFO("Shared System memory: %.2f GiB", memorySizeGib);
                }
            }

            context->device.physicalDevice = physicalDevice[i];
            context->device.graphicsQueueIndex = queueInfo.graphicsFamilyIndex;
            context->device.presentQueueIndex = queueInfo.presentFamilyIndex;
            context->device.transferQueueIndex = queueInfo.transferFamilyIndex;
            // NOTE: set compute index here if needed.

            // Keep a copy of properties, features and memory info for later use.
            context->device.properties = properties;
            context->device.features = features;
            context->device.memory = memory;
            break;
        }
    }

    // Ensure a device was selected
    if (!context->device.physicalDevice) {
        KERROR("No physical devices were found which meet the requirements.");
        return FALSE;
    }

    KINFO("Physical device selected.");
    return TRUE;
}

b8 PhysicalDeviceMeetsRequirements(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties* properties,
    const VkPhysicalDeviceFeatures* features,
    const vulkan_physical_device_requirements* requirements,
    VulkanPhysicalDeviceQueueFamilyInfo* outQueueInfo,
    VulkanSwapchainSupportInfo* outSwapchainSupport) {
    // Evaluate device properties to determine if it meets the needs of our applcation.
    outQueueInfo->graphicsFamilyIndex = -1;
    outQueueInfo->presentFamilyIndex = -1;
    outQueueInfo->computeFamilyIndex = -1;
    outQueueInfo->transferFamilyIndex = -1;

    // Discrete GPU?
    if (requirements->discreteGpu) {
        if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            KINFO("Device is not a discrete GPU, and one is required. Skipping.");
            return FALSE;
        }
    }

    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, 0);
    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    // Look at each queue and see what queues it supports
    KINFO("Graphics | Present | Compute | Transfer | Name");
    u8 minTransferScore = 255;
    for (u32 i = 0; i < queueFamilyCount; ++i) {
        u8 currentTransferScore = 0;

        // Graphics queue?
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            outQueueInfo->graphicsFamilyIndex = i;
            ++currentTransferScore;
        }

        // Compute queue?
        if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            outQueueInfo->computeFamilyIndex = i;
            ++currentTransferScore;
        }

        // Transfer queue?
        if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            // Take the index if it is the current lowest. This increases the
            // liklihood that it is a dedicated transfer queue.
            if (currentTransferScore <= minTransferScore) {
                minTransferScore = currentTransferScore;
                outQueueInfo->transferFamilyIndex = i;
            }
        }

        // Present queue?
        VkBool32 supportsPresent = VK_FALSE;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supportsPresent));
        if (supportsPresent) {
            outQueueInfo->presentFamilyIndex = i;
        }
    }

    // Print out some info about the device
    KINFO("       %d |       %d |       %d |        %d | %s",
          outQueueInfo->graphicsFamilyIndex != -1,
          outQueueInfo->presentFamilyIndex != -1,
          outQueueInfo->computeFamilyIndex != -1,
          outQueueInfo->transferFamilyIndex != -1,
          properties->deviceName);

    if (
        (!requirements->graphics || (requirements->graphics && outQueueInfo->graphicsFamilyIndex != -1)) &&
        (!requirements->present || (requirements->present && outQueueInfo->presentFamilyIndex != -1)) &&
        (!requirements->compute || (requirements->compute && outQueueInfo->computeFamilyIndex != -1)) &&
        (!requirements->transfer || (requirements->transfer && outQueueInfo->transferFamilyIndex != -1))) {
        KINFO("Device meets queue requirements.");
        KTRACE("Graphics Family Index: %i", outQueueInfo->graphicsFamilyIndex);
        KTRACE("Present Family Index:  %i", outQueueInfo->presentFamilyIndex);
        KTRACE("Transfer Family Index: %i", outQueueInfo->transferFamilyIndex);
        KTRACE("Compute Family Index:  %i", outQueueInfo->computeFamilyIndex);

        // Query swapchain support.
        VulkanDeviceQuerySwapchainSupport(
            device,
            surface,
            outSwapchainSupport);

        if (outSwapchainSupport->formatCount < 1 || outSwapchainSupport->presentModeCount < 1) {
            if (outSwapchainSupport->formats) {
                Free(outSwapchainSupport->formats, sizeof(VkSurfaceFormatKHR) * outSwapchainSupport->formatCount, MEMORY_TAG_RENDERER);
            }
            if (outSwapchainSupport->presentModes) {
                Free(outSwapchainSupport->presentModes, sizeof(VkPresentModeKHR) * outSwapchainSupport->presentModeCount, MEMORY_TAG_RENDERER);
            }
            KINFO("Required swapchain support not present, skipping device.");
            return FALSE;
        }

        // Device extensions.
        if (requirements->deviceExtensionNames) {
            u32 availableExtensionsCount = 0;
            VkExtensionProperties* availableExtensions = 0;
            VK_CHECK(vkEnumerateDeviceExtensionProperties(
                device,
                0,
                &availableExtensionsCount,
                0));
            if (availableExtensionsCount != 0) {
                availableExtensions = Allocate(sizeof(VkExtensionProperties) * availableExtensionsCount, MEMORY_TAG_RENDERER);
                VK_CHECK(vkEnumerateDeviceExtensionProperties(
                    device,
                    0,
                    &availableExtensionsCount,
                    availableExtensions));

                u32 requiredExtensionsCount = DarrayLength(requirements->deviceExtensionNames);
                for (u32 i = 0; i < requiredExtensionsCount; ++i) {
                    b8 found = FALSE;
                    for (u32 j = 0; j < availableExtensionsCount; ++j) {
                        if (StringEqual(requirements->deviceExtensionNames[i], availableExtensions[j].extensionName)) {
                            found = TRUE;
                            break;
                        }
                    }

                    if (!found) {
                        KINFO("Required extension not found: '%s', skipping device.", requirements->deviceExtensionNames[i]);
                        Free(availableExtensions, sizeof(VkExtensionProperties) * availableExtensionsCount, MEMORY_TAG_RENDERER);
                        return FALSE;
                    }
                }
            }
            Free(availableExtensions, sizeof(VkExtensionProperties) * availableExtensionsCount, MEMORY_TAG_RENDERER);
        }

        // Sampler anisotropy
        if (requirements->samplerAnisotropy && !features->samplerAnisotropy) {
            KINFO("Device does not support samplerAnisotropy, skipping.");
            return FALSE;
        }

        // Device meets all requirements.
        return TRUE;
    }

    return FALSE;
}