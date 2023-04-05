#include "RendererBackend.h"

#include "vulkan/VulkanBackend.h"

b8 RendererBackendCreate(RendererBackendType type, RendererBackend* outRendererBackend) {

    if (type == RENDERER_BACKEND_TYPE_VULKAN){
        outRendererBackend->initialize = VulkanRendererBackendInitialize;
        outRendererBackend->shutdown = VulkanRendererBackendShutdown;
        outRendererBackend->beginFrame = VulkanRendererBackendBeginFrame;
        outRendererBackend->endFrame = VulkanRendererBackendEndFrame;
        outRendererBackend->resized = VulkanRendererBackendOnResized;

        return true;
    }

    return false;
}

void RendererBackendDestroy(RendererBackend* rendererBackend) {
    rendererBackend->initialize = 0;
    rendererBackend->shutdown = 0;
    rendererBackend->beginFrame = 0;
    rendererBackend->endFrame = 0;
    rendererBackend->resized = 0;
}