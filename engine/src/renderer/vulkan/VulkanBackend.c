#include "VulkanBackend.h"
#include "VulkanTypes.inl"
#include "VulkanPlatform.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanRenderpass.h"
#include "VulkanCommandBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanFence.h"

#include "core/Logger.h"
#include "containers/Darray.h"
#include "core/String.h"
#include "core/Memory.h"
#include "core/Application.h"

#include "platform/Platform.h"

static VulkanContext context;
static u32 cachedFramebufferWidth;
static u32 cachedFramebufferHeight;

VKAPI_ATTR VkBool32 VKAPI_CALL VkDebugCallback (VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData);

i32 FindMemoryIndex(u32 typeFilter, u32 propertyFlags);

void CreateCommandBuffers(RendererBackend* backend);
void RegenerateFramebuffers(RendererBackend* backend, VulkanSwapchain* swapchain, VulkanRenderpass* renderpass);

b8 VulkanRendererBackendInitialize(RendererBackend* backend, const char* applicationName, struct PlatformState* platState) {
    context.FindMemoryIndex = FindMemoryIndex;
    
    // TODO: custom allocator.
    context.allocator = 0;

    ApplicationGetFramebufferSize(&cachedFramebufferWidth, &cachedFramebufferHeight);
    context.framebufferWidth = (cachedFramebufferWidth != 0) ? cachedFramebufferWidth : 800;
    context.framebufferHeight = (cachedFramebufferHeight != 0) ? cachedFramebufferHeight : 600;
    cachedFramebufferWidth = 0;
    cachedFramebufferHeight = 0;

    // Setup Vulkan instance.
    VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.apiVersion = VK_API_VERSION_1_2;
    appInfo.pApplicationName = applicationName;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Kohi Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo createInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    createInfo.pApplicationInfo = &appInfo;

    const char** requiredExtensions = DarrayCreate(const char*);
    DarrayPush(requiredExtensions, &VK_KHR_SURFACE_EXTENSION_NAME);
    PlatformGetRequiredExtensionNames(&requiredExtensions);
#if defined(_DEBUG)
    DarrayPush(requiredExtensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    KDEBUG("Required extensions:");
    u32 length = DarrayLength(requiredExtensions);
    for (u32 i = 0; i < length; ++i)
        KDEBUG(requiredExtensions[i]);
#endif

    createInfo.enabledExtensionCount = DarrayLength(requiredExtensions);
    createInfo.ppEnabledExtensionNames = requiredExtensions;

    const char** requiredValidationLayerNames = 0;
    u32 requiredValidationLayerCount = 0;

#if defined(_DEBUG)
    KINFO("Validation layers enabled. Enumerating...");

    requiredValidationLayerNames = DarrayCreate(const char*);
    DarrayPush(requiredValidationLayerNames, &"VK_LAYER_KHRONOS_validation");
    requiredValidationLayerCount = DarrayLength(requiredValidationLayerNames);

    u32 availableLayerCount = 0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&availableLayerCount, 0));
    VkLayerProperties* availableLayers = DarrayReserve(VkLayerProperties, availableLayerCount);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers));

    for (u32 i = 0; i < requiredValidationLayerCount; ++i) {
        KINFO("Searching for layer : %s...", requiredValidationLayerNames[i]);
        b8 found = FALSE;
        for (u32 j = 0; j < availableLayerCount; ++j) {
            if (StringEqual(requiredValidationLayerNames[i], availableLayers[j].layerName)) {
                found = TRUE;
                KINFO("Found.");
                break;
            }
        }

        if (!found){
            KFATAL("Required validation layer is missing: %s", requiredValidationLayerNames[i]);
            return FALSE;
        }
    }
    KINFO("All required validation layers are present.");
#endif

    createInfo.enabledLayerCount =  requiredValidationLayerCount;
    createInfo.ppEnabledLayerNames = requiredValidationLayerNames;

    VK_CHECK(vkCreateInstance(&createInfo, context.allocator, &context.instance));
    KINFO ("Vulkan Instance created.");

#if defined(_DEBUG)
    KDEBUG("Creating Vulkan debugger...");
    u32 logSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debugCreateInfo.messageSeverity = logSeverity;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debugCreateInfo.pfnUserCallback = VkDebugCallback;

    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkCreateDebugUtilsMessengerEXT");
    KASSERTMSG(func, "Failed to create debug messenger!");
    VK_CHECK(func(context.instance, &debugCreateInfo, context.allocator, &context.debugMessenger));
    KDEBUG("Vulkan debugger created");
#endif

    KDEBUG("Creating Vulkan Surface...");
    if (!PlatformCreateVulkanSurface(platState, &context)) {
        KERROR("Failed to create platform surface!");
        return FALSE;
    }
    KDEBUG("Vulkan surface created.");

    if (!VulkanDeviceCreate(&context)){
        KERROR("Failed to create Device!");
        return FALSE;
    }

    VulkanSwapchainCreate(
        &context,
        context.framebufferWidth,
        context.framebufferHeight,
        &context.swapchain);

    VulkanRenderpassCreate(
        &context,
        &context.mainRenderpass,
        0, 0, context.framebufferWidth, context.framebufferHeight,
        0.0f, 0.0f, 0.2f, 1.0f,
        1.0f, 0);

    // Swapchain framebuffers.
    context.swapchain.framebuffers = DarrayReserve(VulkanFramebuffer, context.swapchain.imageCount);
    RegenerateFramebuffers(backend, &context.swapchain, &context.mainRenderpass);

    CreateCommandBuffers(backend);

    // Create sync objects.
    context.imageAvailableSemaphores = DarrayReserve(VkSemaphore, context.swapchain.maxFramesInFlight);
    context.queueCompleteSemaphores = DarrayReserve(VkSemaphore, context.swapchain.maxFramesInFlight);
    context.inFlightFences = DarrayReserve(VulkanFence, context.swapchain.maxFramesInFlight);

    for (u8 i = 0; i < context.swapchain.maxFramesInFlight; ++i) {
        VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(context.device.logicalDevice, &semaphore_create_info, context.allocator, &context.imageAvailableSemaphores[i]);
        vkCreateSemaphore(context.device.logicalDevice, &semaphore_create_info, context.allocator, &context.queueCompleteSemaphores[i]);

        // Create the fence in a signaled state, indicating that the first frame has already been "rendered".
        // This will prevent the application from waiting indefinitely for the first frame to render since it
        // cannot be rendered until a frame is "rendered" before it.
        VulkanFenceCreate(&context, TRUE, &context.inFlightFences[i]);
    }

    // In flight fences should not yet exist at this point, so clear the list. These are stored in pointers
    // because the initial state should be 0, and will be 0 when not in use. Acutal fences are not owned
    // by this list.
    context.imagesInFlight = DarrayReserve(VulkanFence, context.swapchain.imageCount);
    for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
        context.imagesInFlight[i] = 0;
    }

    KINFO("Vulkan renderer initialized successfully.");
    return TRUE;
}

void VulkanRendererBackendShutdown(RendererBackend* backend) {
    vkDeviceWaitIdle(context.device.logicalDevice);
    // Destroy in the opposite order of creation.

    // Sync objects
    for (u8 i = 0; i < context.swapchain.maxFramesInFlight; ++i) {
        if (context.imageAvailableSemaphores[i]) {
            vkDestroySemaphore(
                context.device.logicalDevice,
                context.imageAvailableSemaphores[i],
                context.allocator);
            context.imageAvailableSemaphores[i] = 0;
        }
        if (context.queueCompleteSemaphores[i]) {
            vkDestroySemaphore(
                context.device.logicalDevice,
                context.queueCompleteSemaphores[i],
                context.allocator);
            context.queueCompleteSemaphores[i] = 0;
        }
        VulkanFenceDestroy(&context, &context.inFlightFences[i]);
    }
    DarrayDestory(context.imageAvailableSemaphores);
    context.imageAvailableSemaphores = 0;

    DarrayDestory(context.queueCompleteSemaphores);
    context.queueCompleteSemaphores = 0;

    DarrayDestory(context.inFlightFences);
    context.inFlightFences = 0;

    DarrayDestory(context.imagesInFlight);
    context.imagesInFlight = 0;

    for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
        if (context.graphicsCommandBuffers[i].handle) {
            VulkanCommandBufferFree(
                &context,
                context.device.graphicsCommandPool,
                &context.graphicsCommandBuffers[i]
                );
            context.graphicsCommandBuffers[i].handle = 0;
        }
    }

    DarrayDestory(context.graphicsCommandBuffers);
    context.graphicsCommandBuffers = 0;

    // Destroy framebuffers
    for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
        VulkanFramebufferDestroy(&context, &context.swapchain.framebuffers[i]);
    }

    VulkanRenderpassDestroy(&context, &context.mainRenderpass);
    VulkanSwapchainDestroy(&context, &context.swapchain);

    KDEBUG("Destroying Vulkan device...");
    VulkanDeviceDestroy(&context);
    
    KDEBUG("Destroying Vulkan surface...");
    if (context.surface) {
        vkDestroySurfaceKHR(context.instance, context.surface, context.allocator);
        context.surface = 0;
    }

    KDEBUG("Destroying vulkan debugger...");
    if (context.debugMessenger) {
        PFN_vkDestroyDebugUtilsMessengerEXT func =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT");
        func(context.instance, context.debugMessenger, context.allocator);
    }

    KDEBUG("Destryoing vulkan instance...");
    vkDestroyInstance(context.instance, context.allocator);
}

void VulkanRendererBackendOnResized(RendererBackend* backend, u16 width, u16 height) {

}

b8 VulkanRendererBackendBeginFrame(RendererBackend* backend, f32 deltaTime) {
    return TRUE;
}

b8 VulkanRendererBackendEndFrame(RendererBackend* backend, f32 deltaTime) {
    return TRUE;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VkDebugCallback (VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData) {
    switch (messageSeverity) {
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            KERROR(callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            KWARNING(callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            KINFO(callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            KTRACE(callbackData->pMessage);
            break;
    }
    return VK_FALSE;
}

i32 FindMemoryIndex(u32 typeFilter, u32 propertyFlags) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(context.device.physicalDevice, &memoryProperties);

    for (u32 i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        // Check each memory type to see if its bit is set to 1.
        if (typeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags) {
            return i;
        }
    }

    KWARNING("Unable to find suitable memory type!");
    return -1;
}


void CreateCommandBuffers(RendererBackend* backend) {
    if (!context.graphicsCommandBuffers) {
        context.graphicsCommandBuffers = DarrayReserve(VulkanCommandBuffer, context.swapchain.imageCount);
        for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
            ZeroMemory(&context.graphicsCommandBuffers[i], sizeof(VulkanCommandBuffer));
        }
    }

    for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
        if (context.graphicsCommandBuffers[i].handle) {
            VulkanCommandBufferFree(
                &context,
                context.device.graphicsCommandPool,
                &context.graphicsCommandBuffers[i]);
        }
        ZeroMemory(&context.graphicsCommandBuffers[i], sizeof(VulkanCommandBuffer));
        VulkanCommandBufferAllocate(
            &context,
            context.device.graphicsCommandPool,
            TRUE,
            &context.graphicsCommandBuffers[i]);
    }

    KDEBUG("Vulkan command buffers created.");
}

void RegenerateFramebuffers(RendererBackend* backend, VulkanSwapchain* swapchain, VulkanRenderpass* renderpass) {
    for (u32 i = 0; i < swapchain->imageCount; ++i) {
        // TODO: make this dynamic based on the currently configured attachments
        u32 attachmentCount = 2;
        VkImageView attachments[] = {
            swapchain->views[i],
            swapchain->depthAttachment.view};

        VulkanFramebufferCreate(
            &context,
            renderpass,
            context.framebufferWidth,
            context.framebufferHeight,
            attachmentCount,
            attachments,
            &context.swapchain.framebuffers[i]);
    }
}