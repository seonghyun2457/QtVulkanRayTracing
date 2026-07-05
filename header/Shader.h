#ifndef SHADER_H
#define SHADER_H

#include <QVulkanInstance>

#include <vector>

class Shader
{
public:
    Shader() = delete;
    virtual ~Shader() = delete;

    Shader(const Shader& iOther) = delete;
    Shader& operator=(const Shader& iOther) = delete;

    Shader(Shader&& iOther) = delete;
    Shader& operator=(Shader&& iOther) = delete;

    static const VkShaderModule createShaderModule(QVulkanInstance* ipVulkanInstance, const VkDevice& iDevice, const std::string& iShaderFilePath);
    static void destroyShaderModule(QVulkanInstance* ipVulkanInstance, const VkDevice& iDevice, VkShaderModule& ioShaderModule);

private:
    static std::vector<char> readShaderFile(const std::string& iShaderFilePath);

private:
};

#endif // SHADER_H
