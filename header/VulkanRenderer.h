#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include <QVulkanInstance>
#include <QVulkanFunctions>

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
    void printVulkanLog(const QString& iString);
    void printDebugLog(const QString& iString);

private:
    void createSurface();
    void createSwapChain();

private:
    // Vulkan Window
    VulkanWindow* m_pWindow{nullptr};

    // Vulkan Instance
    QVulkanInstance m_vulkanInstance;

    // Surface
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};
    VkSurfaceFormatKHR m_surfaceFormat{};

    // - SwapChain
    std::unique_ptr<SwapChain> m_pSwapchain;


private:
    // SUPPORT
    // - Pointer to functions
    QVulkanFunctions* m_pFunctions{VK_NULL_HANDLE};
    QVulkanDeviceFunctions* m_pDeviceFunctions{VK_NULL_HANDLE};
};

#endif // VULKANRENDERER_H
