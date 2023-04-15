#include "VulkanShaderUtils.h"

#include "core/String.h"
#include "core/Logger.h"
#include "core/Memory.h"

#include "platform/Filesystem.h"

b8 CreateShaderModule(
    VulkanContext* context,
    const char* name,
    const char* typeStr,
    VkShaderStageFlagBits shaderStageFlag,
    u32 stageIndex,
    VulkanShaderStage* shaderStage) {
    // Build file name.
    char fileName[512];
    StringFormat(fileName, "assets/shaders/%s.%s.spv", name, typeStr);

    ZeroMemory(&shaderStage[stageIndex].createInfo, sizeof(VkShaderModuleCreateInfo));
    shaderStage[stageIndex].createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    // Obtain file handle.
    FileHandle handle;
    if (!FilesystemOpen(fileName, FILE_MODE_READ, true, &handle)) {
        KERROR("Unable to read shader module: %s.", fileName);
        return false;
    }

    // Read the entire file as binary.
    u64 size = 0;
    u8* fileBuffer = 0;
    if (!FilesystemReadAllBytes(&handle, &fileBuffer, &size)) {
        KERROR("Unable to binary read shader module: %s.", fileName);
        return false;
    }
    shaderStage[stageIndex].createInfo.codeSize = size;
    shaderStage[stageIndex].createInfo.pCode = (u32*)fileBuffer;

    // Close the file.
    FilesystemClose(&handle);

    VK_CHECK(vkCreateShaderModule(
        context->device.logicalDevice,
        &shaderStage[stageIndex].createInfo,
        context->allocator,
        &shaderStage[stageIndex].handle));

    // Shader stage info
    ZeroMemory(&shaderStage[stageIndex].shaderStageCreateInfo, sizeof(VkPipelineShaderStageCreateInfo));
    shaderStage[stageIndex].shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage[stageIndex].shaderStageCreateInfo.stage = shaderStageFlag;
    shaderStage[stageIndex].shaderStageCreateInfo.module = shaderStage[stageIndex].handle;
    shaderStage[stageIndex].shaderStageCreateInfo.pName = "main";

    if (fileBuffer) {
        Free(fileBuffer, sizeof(u8) * size, MEMORY_TAG_STRING);
        fileBuffer = 0;
    }

    return true;
}