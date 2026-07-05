#include "Shader.h"
#include <fstream>

#include <QVulkanDeviceFunctions>
#include <QtAssert>

const VkShaderModule Shader::createShaderModule(QVulkanInstance* ipVulkanInstance, const VkDevice& iDevice, const std::string& iShaderFilePath)
{
    Q_ASSERT(ipVulkanInstance);
    Q_ASSERT(iDevice);

    QVulkanDeviceFunctions* pDeviceFunctions = ipVulkanInstance->deviceFunctions(iDevice);

    // Read in SPIR-V code of shaders
    std::vector<char> shaderCode = readShaderFile(iShaderFilePath);

    VkShaderModuleCreateInfo shaderModuleCreateInfo{};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = shaderCode.size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

    VkShaderModule shaderModule{VK_NULL_HANDLE};
    VkResult result = pDeviceFunctions->vkCreateShaderModule(iDevice, &shaderModuleCreateInfo, nullptr, &shaderModule);
    if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Shader module");

    return shaderModule;
}

void Shader::destroyShaderModule(QVulkanInstance* ipVulkanInstance, const VkDevice& iDevice, VkShaderModule& ioShaderModule)
{
    Q_ASSERT(ipVulkanInstance);
    Q_ASSERT(iDevice);

    QVulkanDeviceFunctions* pDeviceFunctions = ipVulkanInstance->deviceFunctions(iDevice);

    if (ioShaderModule) {
        pDeviceFunctions->vkDestroyShaderModule(iDevice, ioShaderModule, nullptr);
        ioShaderModule = VK_NULL_HANDLE;
    }
}

std::vector<char> Shader::readShaderFile(const std::string& iShaderFilePath)
{
    // Open stream from given file
    // std::ios::binary tells stream to read file as binary
    // std::ios::ate tells stream to start reading from end of file
    std::ifstream file(iShaderFilePath, std::ios_base::binary | std::ios::ate);

    // Check if file stream successfully opened
    if (!file.is_open()) throw std::runtime_error("Failed to open a file");

    // Get current read position and use to resize file buffer
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> fileBuffer(fileSize);

    // Move read position (seek to) the start of the file
    file.seekg(0);

    // Read the file data into the buffer (stream "fileSize" in total)
    file.read(fileBuffer.data(), fileSize);

    return fileBuffer;
}
