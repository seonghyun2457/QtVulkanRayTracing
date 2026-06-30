#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include <QVulkanInstance>
#include <QVulkanFunctions>

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


    bool initialize();

    void cleanup();

private:
    // LOGGING
    void printVulkanLog(const QString& iString);
    void printDebugLog(const QString& iString);

private:
    void createInstance();
    void createSurface();

private:
    // Vulkan Window
    VulkanWindow* m_window{nullptr};

    // Vulkan Instance
    QVulkanInstance m_vulkanInstance;

    // Surface
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};

    // SUPPORT
    // - Pointer to functions
    QVulkanFunctions* m_pFunctions{VK_NULL_HANDLE};
    QVulkanDeviceFunctions* m_pDeviceFunctions{VK_NULL_HANDLE};
};

#endif // VULKANRENDERER_H
