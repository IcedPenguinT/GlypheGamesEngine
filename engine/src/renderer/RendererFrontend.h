#pragma once

#include "RendererTypes.inl"

b8 RendererSystemInitialize(u64* memoryRequirement, void* state, const char* applicationName);
void RendererSystemShutdown(void* state);

void RendererOnResized(u16 width, u16 height);

b8 RendererDrawFrame(RenderPacket* packet);