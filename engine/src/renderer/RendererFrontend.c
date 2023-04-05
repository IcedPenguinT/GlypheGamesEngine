#include "RendererFrontend.h"
#include "RendererBackend.h"

#include "core/Logger.h"
#include "core/Memory.h"

typedef struct RendererSystemState {
    RendererBackend backend;
} RendererSystemState;

static RendererSystemState* statePtr;

b8 RendererSystemInitialize(u64* memoryRequirment, void* state, const char* applicationName) {
    *memoryRequirment = sizeof(RendererSystemState);
    if (state == 0) 
        return true;
    statePtr = state;
    RendererBackendCreate(RENDERER_BACKEND_TYPE_VULKAN, &statePtr->backend);
    //RendererBackendCreate(RENDERER_BACKEND_TYPE_VULKAN, &statePtr->backend);
    statePtr->backend.frameNumber = 0;

    if (!statePtr->backend.initialize(&statePtr->backend, applicationName)) {
        KFATAL("Renderer backend failed to initialize. Shutting down.");
        return false;
    }
    return true;
}

void RendererSystemShutdown(void* state){
    if (statePtr) {
        statePtr->backend.shutdown(&statePtr->backend);
    }
    statePtr = 0;
}

b8 RendererBeginFrame(f32 deltaTime){
    if (!statePtr) {
        return false;
    }
    return statePtr->backend.beginFrame(&statePtr->backend, deltaTime);
}

b8 RendererEndFrame(f32 deltaTime) {
    if (!statePtr) {
        return false;
    }
    b8 result = statePtr->backend.endFrame(&statePtr->backend, deltaTime);
    statePtr->backend.frameNumber++;
    return result;
}

void RendererOnResized(u16 width, u16 height) {
    if (statePtr) {
        statePtr->backend.resized(&statePtr->backend, width, height);
    } else {
        KWARNING("renderer backend does not exist to accept resize: %i %i", width, height);
    }
}

b8 RendererDrawFrame(RenderPacket* packet) {
    if (RendererBeginFrame(packet->deltaTime)){
        b8 result = RendererEndFrame(packet->deltaTime);

        if (!result){
            KERROR("RendererEndFrame failed. Application shutting down...");
            return false;
        }
    }
    return true;
}