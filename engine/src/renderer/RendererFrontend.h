#pragma once

#include "RendererTypes.inl"

struct StaticMeshData;
struct PlatformState;

b8 RendererInitialize(const char* applicationName, struct PlatformState* platState);
void RendererShutdown();

void RendererOnResized(u16 width, u16 height);

b8 RendererDrawFrame(RenderPacket* packet);