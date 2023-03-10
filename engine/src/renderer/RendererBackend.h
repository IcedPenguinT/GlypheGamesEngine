#pragma once

#include "RendererTypes.inl"

struct PlatformState;

b8 RendererBackendCreate(RendererBackendType type, struct PlatformState* platState, RendererBackend* outRendererBackend);
void RendererBackendDestroy(RendererBackend* rendererBackend);