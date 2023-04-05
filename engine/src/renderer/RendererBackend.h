#pragma once

#include "RendererTypes.inl"

struct PlatformState;

b8 RendererBackendCreate(RendererBackendType type, RendererBackend* outRendererBackend);
void RendererBackendDestroy(RendererBackend* rendererBackend);