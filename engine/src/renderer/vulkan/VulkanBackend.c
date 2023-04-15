#include "VulkanBackend.h"
#include "VulkanTypes.inl"
#include "VulkanPlatform.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanRenderpass.h"
#include "VulkanCommandBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanFence.h"
#include "VulkanUtils.h"
#include "VulkanBuffer.h"

#include "core/Logger.h"
#include "containers/Darray.h"
#include "math/MathTypes.h"
#include "core/String.h"
#include "core/Memory.h"
#include "core/Application.h"

#include "platform/Platform.h"
#include "shaders/VulkanObjectShader.h"

static VulkanContext context;
static u32 cachedFramebufferWidth;
static u32 cachedFramebufferHeight;

VKAPI_ATTR VkBool32 VKAPI_CALL VkDebugCallback (VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData);

i32 FindMemoryIndex(u32 typeFilter, u32 propertyFlags);
b8 CreateBuffers(VulkanContext* context);

void CreateCommandBuffers(RendererBackend* backend);
void RegenerateFramebuffers(RendererBackend* backend, VulkanSwapchain* swapchain, VulkanRenderpass* renderpass);
b8 RecreateSwapchain(RendererBackend* backend);

void UploadDataRange(VulkanContext* context, VkCommandPool pool, VkFence fence, VkQueue queue, VulkanBuffer* buffer, u64 offset, u64 size, void* data) {
    // Create a host-visible staging buffer to upload to. Mark it as the source of the transfer.
    VkBufferUsageFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VulkanBuffer staging;
    VulkanBufferCreate(context, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, flags, true, &staging);

    // Load the data into the staging buffer.
    VulkanBufferLoadData(context, &staging, 0, size, 0, data);

    // Perform the copy from staging to the device local buffer.
    VulkanBufferCopyTo(context, pool, fence, queue, staging.handle, 0, buffer->handle, offset, size);

    // Clean up the staging buffer.
    VulkanBufferDestroy(context, &staging);
}

b8 VulkanRendererBackendInitialize(RendererBackend* backend, const char* applicationName) {
    context.findMemoryIndex = FindMemoryIndex;
    
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
        b8 found = false;
        for (u32 j = 0; j < availableLayerCount; ++j) {
            if (StringEqual(requiredValidationLayerNames[i], availableLayers[j].layerName)) {
                found = true;
                KINFO("Found.");
                break;
            }
        }

        if (!found){
            KFATAL("Required validation layer is missing: %s", requiredValidationLayerNames[i]);
            return false;
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
    if (!PlatformCreateVulkanSurface(&context)) {
        KERROR("Failed to create platform surface!");
        return false;
    }
    KDEBUG("Vulkan surface created.");

    if (!VulkanDeviceCreate(&context)){
        KERROR("Failed to create Device!");
        return false;
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
        VulkanFenceCreate(&context, true, &context.inFlightFences[i]);
    }

    // In flight fences should not yet exist at this point, so clear the list. These are stored in pointers
    // because the initial state should be 0, and will be 0 when not in use. Acutal fences are not owned
    // by this list.
    context.imagesInFlight = DarrayReserve(VulkanFence, context.swapchain.imageCount);
    for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
        context.imagesInFlight[i] = 0;
    }

    // Create builtin shaders
    if (!VulkanObjectShaderCreate(&context, &context.objectShader)) {
        KERROR("Error loading built-in basic_lighting shader.");
        return false;
    }

    CreateBuffers(&context);

     // TODO: temporary test code
    const u32 vertCount = 4;
    Vertex3D verts[vertCount];
    ZeroMemory(verts, sizeof(Vertex3D) * vertCount);

    verts[0].position.x = 0.0;
    verts[0].position.y = -0.5;

    verts[1].position.x = 0.5;
    verts[1].position.y = 0.5;

    verts[2].position.x = 0;
    verts[2].position.y = 0.5;

    verts[3].position.x = 0.5;
    verts[3].position.y = -0.5;

    const u32 indexCount = 6;
    u32 indices[indexCount] = {0, 1, 2, 0, 3, 1};

    UploadDataRange(&context, context.device.graphicsCommandPool, 0, context.device.graphicsQueue, &context.objectVertexBuffer, 0, sizeof(Vertex3D) * vertCount, verts);
    UploadDataRange(&context, context.device.graphicsCommandPool, 0, context.device.graphicsQueue, &context.objectIndexBuffer, 0, sizeof(u32) * indexCount, indices);
    // TODO: end temp code

    KINFO("Vulkan renderer initialized successfully.");
    return true;
}

void VulkanRendererBackendShutdown(RendererBackend* backend) {
    vkDeviceWaitIdle(context.device.logicalDevice);
    // Destroy in the opposite order of creation.

    VulkanBufferDestroy(&context, &context.objectVertexBuffer);
    VulkanBufferDestroy(&context, &context.objectIndexBuffer);
    VulkanObjectShaderDestroy(&context, &context.objectShader);
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
    DarrayDestroy(context.imageAvailableSemaphores);
    context.imageAvailableSemaphores = 0;

    DarrayDestroy(context.queueCompleteSemaphores);
    context.queueCompleteSemaphores = 0;

    DarrayDestroy(context.inFlightFences);
    context.inFlightFences = 0;

    DarrayDestroy(context.imagesInFlight);
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

    DarrayDestroy(context.graphicsCommandBuffers);
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
    // Update the "framebuffer size generation", a counter which indicates when the
    // framebuffer size has been updated.
    cachedFramebufferWidth = width;
    cachedFramebufferHeight = height;
    context.framebufferSizeGeneration++;

    KINFO("Vulkan renderer backend->resized: w/h/gen: %i/%i/%llu", width, height, context.framebufferSizeGeneration);
}

b8 VulkanRendererBackendBeginFrame(RendererBackend* backend, f32 deltaTime) {
    VulkanDevice* device = &context.device;

    // Check if recreating swap chain and boot out.
    if (context.recreatingSwapchain) {
        VkResult result = vkDeviceWaitIdle(device->logicalDevice);
        if (!VulkanResultIsSuccess(result)) {
            KERROR("vulkan_RendererBackend_begin_frame vkDeviceWaitIdle (1) failed: '%s'", VulkanResultString(result, true));
            return false;
        }
        KINFO("Recreating swapchain, booting.");
        return false;
    }

    // Check if the framebuffer has been resized. If so, a new swapchain must be created.
    if (context.framebufferSizeGeneration != context.framebufferSizeLastGeneration) {
        VkResult result = vkDeviceWaitIdle(device->logicalDevice);
        if (!VulkanResultIsSuccess(result)) {
            KERROR("vulkan_RendererBackend_begin_frame vkDeviceWaitIdle (2) failed: '%s'", VulkanResultString(result, true));
            return false;
        }

        // If the swapchain recreation failed (because, for example, the window was minimized),
        // boot out before unsetting the flag.
        if (!RecreateSwapchain(backend)) {
            return false;
        }

        KINFO("Resized, booting.");
        return false;
    }

    // Wait for the execution of the current frame to complete. The fence being free will allow this one to move on.
    if (!VulkanFenceWait(
            &context,
            &context.inFlightFences[context.currentFrame],
            UINT64_MAX)) {
        KWARNING("In-flight fence wait failure!");
        return false;
    }

    // Acquire the next image from the swap chain. Pass along the semaphore that should signaled when this completes.
    // This same semaphore will later be waited on by the queue submission to ensure this image is available.
    if (!VulkanSwapchainAcquireNextImageIndex(
            &context,
            &context.swapchain,
            UINT64_MAX,
            context.imageAvailableSemaphores[context.currentFrame],
            0,
            &context.imageIndex)) {
        return false;
    }

    // Begin recording commands.
    VulkanCommandBuffer* commandBuffer = &context.graphicsCommandBuffers[context.imageIndex];
    VulkanCommandBufferReset(commandBuffer);
    VulkanCommandBufferBegin(commandBuffer, false, false, false);

    // Dynamic state
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (f32)context.framebufferHeight;
    viewport.width = (f32)context.framebufferWidth;
    viewport.height = -(f32)context.framebufferHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Scissor
    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = context.framebufferWidth;
    scissor.extent.height = context.framebufferHeight;

    vkCmdSetViewport(commandBuffer->handle, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer->handle, 0, 1, &scissor);

    context.mainRenderpass.w = context.framebufferWidth;
    context.mainRenderpass.h = context.framebufferHeight;

    // Begin the render pass.
    VulkanRenderpassBegin(
        commandBuffer,
        &context.mainRenderpass,
        context.swapchain.framebuffers[context.imageIndex].handle);

    // TODO: temporary test code
    VulkanObjectShaderUse(&context, &context.objectShader);

    // Bind vertex buffer at offset.
    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(commandBuffer->handle, 0, 1, &context.objectVertexBuffer.handle, (VkDeviceSize*)offsets);

    // Bind index buffer at offset.
    vkCmdBindIndexBuffer(commandBuffer->handle, context.objectIndexBuffer.handle, 0, VK_INDEX_TYPE_UINT32);

    // Issue the draw.
    vkCmdDrawIndexed(commandBuffer->handle, 6, 1, 0, 0, 0);
    // TODO: end temporary test code

    return true;
}

b8 VulkanRendererBackendEndFrame(RendererBackend* backend, f32 deltaTime) {
    VulkanCommandBuffer* commandBuffer = &context.graphicsCommandBuffers[context.imageIndex];

    // End renderpass
    VulkanRenderpassEnd(commandBuffer, &context.mainRenderpass);

    VulkanCommandBufferEnd(commandBuffer);

    // Make sure the previous frame is not using this image (i.e. its fence is being waited on)
    if (context.imagesInFlight[context.imageIndex] != VK_NULL_HANDLE) {  // was frame
        VulkanFenceWait(
            &context,
            context.imagesInFlight[context.imageIndex],
            UINT64_MAX);
    }

    // Mark the image fence as in-use by this frame.
    context.imagesInFlight[context.imageIndex] = &context.inFlightFences[context.currentFrame];

    // Reset the fence for use on the next frame
    VulkanFenceReset(&context, &context.inFlightFences[context.currentFrame]);

    // Submit the queue and wait for the operation to complete.
    // Begin queue submission
    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};

    // Command buffer(s) to be executed.
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer->handle;

    // The semaphore(s) to be signaled when the queue is complete.
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &context.queueCompleteSemaphores[context.currentFrame];

    // Wait semaphore ensures that the operation cannot begin until the image is available.
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &context.imageAvailableSemaphores[context.currentFrame];

    // Each semaphore waits on the corresponding pipeline stage to complete. 1:1 ratio.
    // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsequent colour attachment
    // writes from executing until the semaphore signals (i.e. one frame is presented at a time)
    VkPipelineStageFlags flags[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.pWaitDstStageMask = flags;

    VkResult result = vkQueueSubmit(
        context.device.graphicsQueue,
        1,
        &submitInfo,
        context.inFlightFences[context.currentFrame].handle);
    if (result != VK_SUCCESS) {
        KERROR("vkQueueSubmit failed with result: %s", VulkanResultString(result, true));
        return false;
    }

    VulkanCommandBufferUpdateSubmitted(commandBuffer);
    // End queue submission

    // Give the image back to the swapchain.
    VulkanSwapchainPresent(
        &context,
        &context.swapchain,
        context.device.graphicsQueue,
        context.device.presentQueue,
        context.queueCompleteSemaphores[context.currentFrame],
        context.imageIndex);


    
    return true;
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
            true,
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

b8 RecreateSwapchain(RendererBackend* backend) {
    // If already being recreated, do not try again.
    if (context.recreatingSwapchain) {
        KDEBUG("RecreateSwapchain called when already recreating. Booting.");
        return false;
    }

    // Detect if the window is too small to be drawn to
    if (context.framebufferWidth == 0 || context.framebufferHeight == 0) {
        KDEBUG("RecreateSwapchain called when window is < 1 in a dimension. Booting.");
        return false;
    }

    // Mark as recreating if the dimensions are valid.
    context.recreatingSwapchain = true;

    // Wait for any operations to complete.
    vkDeviceWaitIdle(context.device.logicalDevice);

    // Clear these out just in case.
    for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
        context.imagesInFlight[i] = 0;
    }

    // Requery support
    VulkanDeviceQuerySwapchainSupport(
        context.device.physicalDevice,
        context.surface,
        &context.device.swapchainSupport);
    VulkanDeviceDetectDepthFormat(&context.device);

    VulkanSwapchainRecreate(
        &context,
        cachedFramebufferWidth,
        cachedFramebufferHeight,
        &context.swapchain);

    // Sync the framebuffer size with the cached sizes.
    context.framebufferWidth = cachedFramebufferWidth;
    context.framebufferHeight = cachedFramebufferHeight;
    context.mainRenderpass.w = context.framebufferWidth;
    context.mainRenderpass.h = context.framebufferHeight;
    cachedFramebufferWidth = 0;
    cachedFramebufferHeight = 0;

    // Update framebuffer size generation.
    context.framebufferSizeLastGeneration = context.framebufferSizeGeneration;

    // cleanup swapchain
    for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
        VulkanCommandBufferFree(&context, context.device.graphicsCommandPool, &context.graphicsCommandBuffers[i]);
    }

    // Framebuffers.
    for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
        VulkanFramebufferDestroy(&context, &context.swapchain.framebuffers[i]);
    }

    context.mainRenderpass.x = 0;
    context.mainRenderpass.y = 0;
    context.mainRenderpass.w = context.framebufferWidth;
    context.mainRenderpass.h = context.framebufferHeight;

    RegenerateFramebuffers(backend, &context.swapchain, &context.mainRenderpass);

    CreateCommandBuffers(backend);

    // Clear the recreating flag.
    context.recreatingSwapchain = false;

    return true;
}

b8 CreateBuffers(VulkanContext* context) {
    VkMemoryPropertyFlagBits memoryPropertyFlag = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    const u64 vertexBufferSize = sizeof(Vertex3D) * 1024 * 1024;
    if (!VulkanBufferCreate(
            context,
            vertexBufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            memoryPropertyFlag,
            true,
            &context->objectVertexBuffer)) {
        KERROR("Error creating vertex buffer.");
        return false;
    }
    context->geometryVertexOffset = 0;

    const u64 indexBufferSize = sizeof(u32) * 1024 * 1024;
    if (!VulkanBufferCreate(
            context,
            indexBufferSize,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            memoryPropertyFlag,
            true,
            &context->objectIndexBuffer)) {
        KERROR("Error creating vertex buffer.");
        return false;
    }
    context->geometryIndexOffset = 0;

    return true;
}