#pragma once

#include "renderer/RendererBackend.h"

b8 VulkanRendererBackendInitialize(RendererBackend* backend, const char* applicationName, struct PlatformState* platState);
void VulkanRendererBackendShutdown(RendererBackend* backend);

void VulkanRendererBackendOnResized(RendererBackend* backend, u16 width, u16 height);

b8 VulkanRendererBackendBeginFrame(RendererBackend* backend, f32 deltaTime);
b8 VulkanRendererBackendEndFrame(RendererBackend* backend, f32 deltaTime);