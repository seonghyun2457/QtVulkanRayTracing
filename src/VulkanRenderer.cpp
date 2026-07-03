#include "VulkanRenderer.h"

#include "VulkanWindow.h"

#include <QDebug>
#include <QVariant>

#include <exception>

VulkanRenderer::VulkanRenderer(VulkanWindow* iWindow) noexcept
    : m_pWindow(iWindow)
    , m_pGraphicDevice(std::make_unique<GraphicDevice>(iWindow))
    , m_pSwapchain(std::make_unique<SwapChain>())
{

}

VulkanRenderer::~VulkanRenderer()
{
    cleanup();
}

bool VulkanRenderer::initializeResources()
{
    bool initialized = true;
    try
    {
        createSurface();
        m_pGraphicDevice->createGraphicDevice(m_vulkanInstance, m_surface);
        createSwapChain();

    } catch (const std::runtime_error& e) {
        printDebugLog(e.what());
        initialized = false;
    }

    return initialized;
}

void VulkanRenderer::cleanup()
{
    // Destroy SwapChain
    destroySwapchain();

    // Destroy Graphic Device
    m_pGraphicDevice->destroy(m_vulkanInstance);
}

void VulkanRenderer::printVulkanLog(const QString& iString) const
{
    emit m_pWindow->vulkanLogSent(iString);
}

void VulkanRenderer::printDebugLog(const QString& iString) const
{
    emit m_pWindow->debugLogSent(iString);
}

bool VulkanRenderer::createVulkanInstance()
{
    printDebugLog("Create vulkan Instance");

    m_vulkanInstance.setApiVersion(m_vulkanInstance.supportedApiVersion());
    m_vulkanInstance.setLayers({ "VK_LAYER_KHRONOS_validation" });

    if (!m_vulkanInstance.create()) {
        qCritical() << "Failed to create QVulkanInstance, error code:" << m_vulkanInstance.errorCode();
        return false;
    }

    printVulkanLog("Extensions:");
    for (const auto& extension : m_vulkanInstance.extensions()) {
        printVulkanLog(extension);
    }

    printVulkanLog("Vulkan api version: " + m_vulkanInstance.apiVersion().toString());

    m_pWindow->setVulkanInstance(&m_vulkanInstance);

    return true;
}

void VulkanRenderer::createSurface()
{

    printDebugLog("Create Surface");

    // Get VkSurfaceKHR info from QWindow
    m_surface = m_vulkanInstance.surfaceForWindow(m_pWindow);

    if (m_surface == nullptr) throw std::runtime_error("Failed to create surface");
}



void VulkanRenderer::createSwapChain()
{

}

void VulkanRenderer::destroySwapchain()
{

}
