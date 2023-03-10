#pragma once

#include "Defines.h"
#include "core/Asserts.h"

#include <vulkan/vulkan.h>

#define VK_CHECK(expr)                 \
    {                                  \
        KASSERT(expr == VK_SUCCESS);   \
    }                                  

typedef struct VulkanContext {
    VkInstance instance;
    VkAllocationCallbacks* allocator;

#if defined(_DEBUG)
    VkDebugUtilsMessengerEXT debugMessenger;
#endif
} VulkanContext;