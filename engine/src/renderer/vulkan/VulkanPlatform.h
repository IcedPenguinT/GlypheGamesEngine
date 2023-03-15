#pragma once

#include "Defines.h"

struct PlatformState;
struct VulkanContext;

b8 PlatformCreateVulkanSurface(struct PlatformState* platState, struct VulkanContext* context);

void PlatformGetRequiredExtensionNames(const char*** namesDarray);