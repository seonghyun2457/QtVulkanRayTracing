#ifndef SHADER_H
#define SHADER_H

#include <QVulkanInstance>

#include "GraphicDevice.h"

#include <vector>

class GraphicDevice;

class Shader
{
public:
    Shader() = delete;
    virtual ~Shader() = delete;

    Shader(const Shader& iOther) = delete;
    Shader& operator=(const Shader& iOther) = delete;

    Shader(Shader&& iOther) = delete;
    Shader& operator=(Shader&& iOther) = delete;

    static const VkShaderModule createShaderModule(GraphicDevice* ipGraphicDevice, const std::string& iShaderFilePath);
    static void destroyShaderModule(GraphicDevice* ipGraphicDevice, VkShaderModule& ioShaderModule);

private:
    static std::vector<char> readShaderFile(const std::string& iShaderFilePath);

private:
};

#endif // SHADER_H
