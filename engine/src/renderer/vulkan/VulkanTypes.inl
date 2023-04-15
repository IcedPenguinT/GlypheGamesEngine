#pragma once

#include "Defines.h"
#include "core/Asserts.h"

#include <vulkan/vulkan.h>

#define VK_CHECK(expr)                 \
    {                                  \
        KASSERT(expr == VK_SUCCESS);   \
    }                                  

typedef struct VulkanBuffer {
    u64 totalSize;
    VkBuffer handle;
    VkBufferUsageFlagBits usage;
    b8 isLocked;
    VkDeviceMemory memory;
    i32 memoryIndex;
    u32 memoryPropertyFlags;
} VulkanBuffer;

typedef struct VulkanSwapchainSupportInfo {
    VkSurfaceCapabilitiesKHR capabilities;
    u32 formatCount;
    VkSurfaceFormatKHR* formats;
    u32 presentModeCount;
    VkPresentModeKHR* presentModes;
} VulkanSwapchainSupportInfo;

typedef struct VulkanDevice {
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    VulkanSwapchainSupportInfo swapchainSupport;
    i32 graphicsQueueIndex;
    i32 presentQueueIndex;
    i32 transferQueueIndex;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;

    VkCommandPool graphicsCommandPool;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;

    VkFormat depthFormat;
} VulkanDevice;

typedef struct VulkanImage {
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    u32 width;
    u32 height;
} VulkanImage;

typedef enum VulkanRenderPassState {
    READY,
    RECORDING,
    IN_RENDER_PASS,
    RECORDING_ENDED,
    SUBMITTED,
    NOT_ALLOCATED
} VulkanRenderPassState;

typedef struct VulkanRenderpass {
    VkRenderPass handle;
    f32 x, y, w, h;
    f32 r, g, b, a;

    f32 depth;
    u32 stencil;

    VulkanRenderPassState state;
} VulkanRenderpass;

typedef struct VulkanFramebuffer {
    VkFramebuffer handle;
    u32 attachmentCount;
    VkImageView* attachments;
    VulkanRenderpass* renderpass;
} VulkanFramebuffer;

typedef struct VulkanSwapchain {
    VkSurfaceFormatKHR imageFormat;
    u8 maxFramesInFlight;
    VkSwapchainKHR handle;
    u32 imageCount;
    VkImage* images;
    VkImageView* views;

    VulkanImage depthAttachment;

    VulkanFramebuffer* framebuffers;
} VulkanSwapchain;

typedef enum VulkanCommandBufferState {
    COMMAND_BUFFER_STATE_READY,
    COMMAND_BUFFER_STATE_RECORDING,
    COMMAND_BUFFER_STATE_IN_RENDER_PASS,
    COMMAND_BUFFER_STATE_RECORDING_ENDED,
    COMMAND_BUFFER_STATE_SUBMITTED,
    COMMAND_BUFFER_STATE_NOT_ALLOCATED
} VulkanCommandBufferState;

typedef struct VulkanCommandBuffer {
    VkCommandBuffer handle;

    // Command buffer state.
    VulkanCommandBufferState state;
} VulkanCommandBuffer;

typedef struct VulkanFence {
    VkFence handle;
    b8 isSignaled;
} VulkanFence;

typedef struct VulkanShaderStage {
    VkShaderModuleCreateInfo createInfo;
    VkShaderModule handle;
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
} VulkanShaderStage;

typedef struct VulkanPipeline {
    VkPipeline handle;
    VkPipelineLayout pipelineLayout;
} VulkanPipeline;

#define OBJECT_SHADER_STAGE_COUNT 2
typedef struct VulkanObjectShader {
    // vertex, fragment
    VulkanShaderStage stages[OBJECT_SHADER_STAGE_COUNT];

    VulkanPipeline pipeline;


} VulkanObjectShader;

typedef struct VulkanContext {
    u32 framebufferWidth;
    u32 framebufferHeight;

    // Current generation of framebuffer size. If it does not match framebuffer_size_lastGeneration,
    // a new one should be generated.
    u64 framebufferSizeGeneration;

    // The generation of the framebuffer when it was last created. Set to framebufferSizeGeneration
    // when updated.
    u64 framebufferSizeLastGeneration;

    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkSurfaceKHR surface;

#if defined(_DEBUG)
    VkDebugUtilsMessengerEXT debugMessenger;
#endif

    VulkanDevice device;
    VulkanSwapchain swapchain;
    u32 imageIndex;
    u32 currentFrame;

    b8 recreatingSwapchain;

    VulkanObjectShader objectShader;

    VulkanRenderpass mainRenderpass;

    VulkanBuffer objectVertexBuffer;
    VulkanBuffer objectIndexBuffer;

    VulkanCommandBuffer* graphicsCommandBuffers;

        // darray
    VkSemaphore* imageAvailableSemaphores;

    // darray
    VkSemaphore* queueCompleteSemaphores;

    u32 inFlightFenceCount;
    VulkanFence* inFlightFences;

    // Holds pointers to fences which exist and are owned elsewhere.
    VulkanFence** imagesInFlight;

    u64 geometryVertexOffset;
    u64 geometryIndexOffset;

    i32 (*findMemoryIndex)(u32 typeFilter, u32 propertyFlags);
} VulkanContext;