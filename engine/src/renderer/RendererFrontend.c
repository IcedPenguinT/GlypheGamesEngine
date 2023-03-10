#include "RendererFrontend.h"
#include "RendererBackend.h"

#include "core/Logger.h"
#include "core/Memory.h"

static RendererBackend* backend = 0;

b8 RendererInitialize(const char* applicationName, struct PlatformState* platState) {
    backend = Allocate(sizeof(RendererBackend), MEMORY_TAG_RENDERER);

    RendererBackendCreate(RENDERER_BACKEND_TYPE_VULKAN, platState, backend);
    backend->frameNumber = 0;

    if (!backend->initialize(backend, applicationName, platState)) {
        KFATAL("Renderer backend failed to initialize. Shutting down.");
        return FALSE;
    }
    return TRUE;
}

void RendererShutdown(){
    backend->shutdown(backend);
    Free(backend, sizeof(RendererBackend), MEMORY_TAG_RENDERER);
}

b8 RendererBeginFrame(f32 deltaTime){
    return backend->beginFrame(backend, deltaTime);
}

b8 RendererEndFrame(f32 deltaTime) {
    b8 result = backend->endFrame(backend, deltaTime);
    backend->frameNumber++;
    return result;
}

void RendererOnResized(u16 width, u16 height) {
    
}

b8 RendererDrawFrame(RenderPacket* packet) {
    if (RendererBeginFrame(packet->deltaTime)){
        b8 result = RendererEndFrame(packet->deltaTime);

        if (!result){
            KERROR("RendererEndFrame failed. Application shutting down...");
            return FALSE;
        }
    }
    return TRUE;
}