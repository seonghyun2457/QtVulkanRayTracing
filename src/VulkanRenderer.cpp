#include "VulkanRenderer.h"

#include "VulkanWindow.h"

#include <QDebug>
#include <QVariant>

#include <exception>

VulkanRenderer::VulkanRenderer(VulkanWindow* iWindow) noexcept
    : m_pWindow(iWindow)
    , m_graphicDevice(nullptr)
    , m_swapChain(nullptr)
{

}

VulkanRenderer::~VulkanRenderer()
{
    cleanup();
    qDebug() << "Destroyed VulkanRenderer";
}

bool VulkanRenderer::initializeResources()
{
    bool initialized = true;
    try
    {
        createSurface();
        createGraphicDevice();
        createSwapChain();

    } catch (const std::runtime_error& e) {
        printDebugLog(e.what());
        initialized = false;
    }

    return initialized;
}

void VulkanRenderer::cleanup()
{
    QVulkanInstance* pVulkanInstance = m_pWindow->vulkanInstance();

    // Wait until Idle status
    if (m_graphicDevice && pVulkanInstance && pVulkanInstance->isValid() && m_graphicDevice && m_graphicDevice->getDevice()) {
        QVulkanDeviceFunctions* pFunctions = pVulkanInstance->deviceFunctions(m_graphicDevice->getDevice());
        pFunctions->vkDeviceWaitIdle(m_graphicDevice->getDevice());
    } else {
        qDebug() << "Vulkan instance doesn't exist";
    }

    // Destroy SwapChain
    if (m_swapChain) {
        m_swapChain = nullptr;
    }

    // Destroy Graphic Device
    if (m_graphicDevice) {
        m_graphicDevice = nullptr;
    };

    qDebug() << "Cleaned up";
}

void VulkanRenderer::recreateImageDependentResources()
{
    printDebugLog("Recreate image dependent resources");

    QVulkanInstance* pVulkanInstance = m_pWindow->vulkanInstance();
    QVulkanDeviceFunctions* pDeviceFunctions = pVulkanInstance->deviceFunctions(m_graphicDevice->getDevice());

    // Wait until idle status
    pDeviceFunctions->vkDeviceWaitIdle(m_graphicDevice->getDevice());

    const size_t prevSwapChainImageCount = m_swapChain->getSwapchainImageCount();
    m_swapChain->recreateSwapchain(m_surface, m_surfaceFormat);

    if (prevSwapChainImageCount != m_swapChain->getSwapchainImageCount()) {
        // Destroy resources
        {

        }

        // Recreate resources
        {

        }
    }
}

void VulkanRenderer::printVulkanLog(const QString& iString) const
{
    emit m_pWindow->vulkanLogSent(iString);
}

void VulkanRenderer::printDebugLog(const QString& iString) const
{
    emit m_pWindow->debugLogSent(iString);
}

void VulkanRenderer::createSurface()
{
    printDebugLog("Create Surface");

    // Get VkSurfaceKHR info from QWindow
    QVulkanInstance* pVulkanInstance = m_pWindow->vulkanInstance();

    if (pVulkanInstance == nullptr) throw std::runtime_error("Vulkan Instance isn't set in Window.");

    m_surface = pVulkanInstance->surfaceForWindow(m_pWindow);
    if (m_surface == nullptr) throw std::runtime_error("Failed to create surface.");
}

void VulkanRenderer::createGraphicDevice()
{
    Q_ASSERT(m_pWindow);
    Q_ASSERT(m_surface);

    m_graphicDevice = std::make_unique<GraphicDevice>(m_pWindow, m_surface);

    if (m_graphicDevice == nullptr) throw std::runtime_error("Failed to create GraphicDevice instance");

    m_graphicDevice->createGraphicDevice();
}



void VulkanRenderer::createSwapChain()
{
    Q_ASSERT(m_pWindow);
    Q_ASSERT(m_surface);

    m_swapChain = std::make_unique<SwapChain>(m_pWindow, m_graphicDevice->getPhysicalDevice(), m_graphicDevice->getDevice());

    m_swapChain->createSwapchain(m_surface, m_surfaceFormat);
}

void VulkanRenderer::recreateImageDependentResources()
{

}
