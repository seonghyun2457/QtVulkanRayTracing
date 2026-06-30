#include "VulkanRenderer.h"

#include "VulkanWindow.h"

#include <QDebug>

#include <exception>

VulkanRenderer::VulkanRenderer(VulkanWindow* iWindow) noexcept
    : m_window(iWindow)
{

}

VulkanRenderer::~VulkanRenderer()
{
    cleanup();
}

bool VulkanRenderer::initialize()
{
    bool initialized = true;
    try
    {
        createInstance();
        createSurface();



    } catch (const std::runtime_error& e) {
        initialized = false;
    }

    return initialized;
}

void VulkanRenderer::cleanup()
{

}

void VulkanRenderer::printVulkanLog(const QString& iString)
{

}

void VulkanRenderer::printDebugLog(const QString& iString)
{

}

void VulkanRenderer::createInstance()
{
    printDebugLog("Create Instance");

    m_vulkanInstance.setApiVersion(m_vulkanInstance.supportedApiVersion());
    m_vulkanInstance.setLayers({ "VK_LAYER_KHRONOS_validation" });

    if (!m_vulkanInstance.create()) {
        qCritical() << "Failed to create QVulkanInstance, error code:" << m_vulkanInstance.errorCode();
        return;
    }

    printVulkanLog("Extensions:");
    for (const auto& extension : m_vulkanInstance.extensions()) {
        printVulkanLog(extension);
    }

    printVulkanLog("Vulkan api version: " + m_vulkanInstance.apiVersion().toString());

    m_window->setVulkanInstance(&m_vulkanInstance);

    m_pFunctions = m_vulkanInstance.functions();

    Q_ASSERT(m_pFunctions != VK_NULL_HANDLE);
}

void VulkanRenderer::createSurface()
{

    printDebugLog("Create Surface");

    // Get VkSurfaceKHR info from QWindow
    m_surface = m_vulkanInstance.surfaceForWindow(m_window);
}
