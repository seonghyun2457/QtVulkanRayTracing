#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include <QVulkanInstance>
#include <QVulkanFunctions>

#include "GraphicDevice.h"
#include "SwapChain.h"

#include <glm/glm.hpp>

class VulkanWindow;

class VulkanRenderer
{
public:
    explicit VulkanRenderer(VulkanWindow* iWindow) noexcept;
    virtual ~VulkanRenderer();

    VulkanRenderer(const VulkanRenderer& iOther) = delete;
    VulkanRenderer& operator=(const VulkanRenderer& iOther) = delete;

    VulkanRenderer(VulkanRenderer&& iOther) = delete;
    VulkanRenderer& operator=(VulkanRenderer&& iOther) = delete;

    bool initializeResources();
    void cleanup();

    void recreateImageDependentResources();

private:
    // LOGGING
    void printVulkanLog(const QString& iString) const;
    void printDebugLog(const QString& iString) const;

private:
    void createSurface();
    void createGraphicDevice();
    void createSwapChain();

private:
    // Vulkan Window
    VulkanWindow* m_pWindow{nullptr};

    // Surface
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};
    VkSurfaceFormatKHR m_surfaceFormat{};

    // Graphic Device
    std::unique_ptr<GraphicDevice> m_graphicDevice{nullptr};

    // SwapChain
    std::unique_ptr<SwapChain> m_swapChain{nullptr};
};

#endif // VULKANRENDERER_H
