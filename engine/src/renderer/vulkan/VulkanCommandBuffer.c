#include "VulkanCommandBuffer.h"

#include "core/Memory.h"

void VulkanCommandBufferAllocate(
    VulkanContext* context,
    VkCommandPool pool,
    b8 isPrimary,
    VulkanCommandBuffer* outCommandBuffer) {

    ZeroMemory(outCommandBuffer, sizeof(outCommandBuffer));

    VkCommandBufferAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocateInfo.commandPool = pool;
    allocateInfo.level = isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocateInfo.commandBufferCount = 1;
    allocateInfo.pNext = 0;

    outCommandBuffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
    VK_CHECK(vkAllocateCommandBuffers(
        context->device.logicalDevice,
        &allocateInfo,
        &outCommandBuffer->handle));
    outCommandBuffer->state = COMMAND_BUFFER_STATE_READY;
}

void VulkanCommandBufferFree(
    VulkanContext* context,
    VkCommandPool pool,
    VulkanCommandBuffer* commandBuffer) {
    vkFreeCommandBuffers(
        context->device.logicalDevice,
        pool,
        1,
        &commandBuffer->handle);

    commandBuffer->handle = 0;
    commandBuffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
}

void VulkanCommandBufferBegin(
    VulkanCommandBuffer* commandBuffer,
    b8 isSingleUse,
    b8 isRenderpassContinue,
    b8 isSimultaneousUse) {

    VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = 0;
    if (isSingleUse) {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }
    if (isRenderpassContinue) {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    }
    if (isSimultaneousUse) {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    }

    VK_CHECK(vkBeginCommandBuffer(commandBuffer->handle, &beginInfo));
    commandBuffer->state = COMMAND_BUFFER_STATE_RECORDING;
}

void VulkanCommandBufferEnd(VulkanCommandBuffer* commandBuffer) {
    VK_CHECK(vkEndCommandBuffer(commandBuffer->handle));
    commandBuffer->state = COMMAND_BUFFER_STATE_RECORDING_ENDED;
}

void VulkanCommandBufferUpdateSubmitted(VulkanCommandBuffer* commandBuffer) {
    commandBuffer->state = COMMAND_BUFFER_STATE_SUBMITTED;
}

void VulkanCommandBufferReset(VulkanCommandBuffer* commandBuffer) {
    commandBuffer->state = COMMAND_BUFFER_STATE_READY;
}

void VulkanCommandBufferAlocateAndBeginSingleUse(
    VulkanContext* context,
    VkCommandPool pool,
    VulkanCommandBuffer* outCommandBuffer) {
    VulkanCommandBufferAllocate(context, pool, TRUE, outCommandBuffer);
    VulkanCommandBufferBegin(outCommandBuffer, TRUE, FALSE, FALSE);
}

void VulkanCommandBufferEndSingleUse(
    VulkanContext* context,
    VkCommandPool pool,
    VulkanCommandBuffer* commandBuffer,
    VkQueue queue) {

    // End the command buffer.
    VulkanCommandBufferEnd(commandBuffer);

    // Submit the queue
    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer->handle;
    VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, 0));

    // Wait for it to finish
    VK_CHECK(vkQueueWaitIdle(queue));

    // Free the command buffer.
    VulkanCommandBufferFree(context, pool, commandBuffer);
 }