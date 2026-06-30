#include "VulkanWindow.h"

#include <QtAssert>

VulkanWindow::VulkanWindow()
    : QWindow()
{
    setSurfaceType(QWindow::VulkanSurface);
}

VulkanWindow::~VulkanWindow()
{
    m_vulkanRenderer = nullptr;
}

void VulkanWindow::createRenderer()
{
    m_vulkanRenderer = std::make_unique<VulkanRenderer>(this);

    if (!m_vulkanRenderer->initialize()) {
        m_vulkanRenderer = nullptr;
    }
}
