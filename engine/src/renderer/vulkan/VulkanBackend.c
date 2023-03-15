#include "VulkanBackend.h"
#include "VulkanTypes.inl"
#include "VulkanPlatform.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"

#include "core/Logger.h"
#include "containers/Darray.h"
#include "core/String.h"

#include "platform/Platform.h"

static VulkanContext context;

VKAPI_ATTR VkBool32 VKAPI_CALL VkDebugCallback (VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData);

i32 FindMemoryIndex(u32 typeFilter, u32 propertyFlags);



b8 VulkanRendererBackendInitialize(RendererBackend* backend, const char* applicationName, struct PlatformState* platState) {
    context.FindMemoryIndex = FindMemoryIndex;
    
    // TODO: custom allocator.
    context.allocator = 0;

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

    KINFO("Vulkan renderer initialized successfully.");
    return TRUE;
}

void VulkanRendererBackendShutdown(RendererBackend* backend) {
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