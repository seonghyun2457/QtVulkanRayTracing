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

    bool createVulkanInstance();
    bool initializeResources();

    void cleanup();

private:
    // LOGGING
    void printVulkanLog(const QString& iString) const;
    void printDebugLog(const QString& iString) const;

private:
    void createSurface();
    void createSwapChain();
    void destroySwapchain();

private:
    // Vulkan Window
    VulkanWindow* m_pWindow{nullptr};

    // Vulkan Instance
    QVulkanInstance m_vulkanInstance;

    // Surface
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};
    VkSurfaceFormatKHR m_surfaceFormat{};

    // Graphic Device
    std::unique_ptr<GraphicDevice> m_pGraphicDevice{nullptr};

    // SwapChain
    std::unique_ptr<SwapChain> m_pSwapchain{nullptr};
};

#endif // VULKANRENDERER_H
