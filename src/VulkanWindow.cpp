#include "VulkanWindow.h"

#include <QDebug>
#include <QtAssert>

VulkanWindow::VulkanWindow()
    : QWindow()
    , m_vulkanRenderer(std::make_unique<VulkanRenderer>(this))
{
    setSurfaceType(QWindow::VulkanSurface);
}

VulkanWindow::~VulkanWindow()
{
    m_vulkanRenderer = nullptr;
}

bool VulkanWindow::createVulkanInstance()
{
    Q_ASSERT(m_vulkanRenderer != nullptr);
    return m_vulkanRenderer->createVulkanInstance();
}

void VulkanWindow::initializeRenderer()
{
    emit debugLogSent("initialize renderer");

    Q_ASSERT(m_vulkanRenderer != nullptr);
    m_initialized = m_vulkanRenderer->initializeResources();
}

void VulkanWindow::exposeEvent(QExposeEvent* event)
{
    emit debugLogSent("exposed");
    qDebug() << "exposed";

    if (!m_initialized && isExposed()) {
        initializeRenderer();
    }
    QWindow::exposeEvent(event);
}

bool VulkanWindow::event(QEvent* event)
{
    emit debugLogSent("event");
    qDebug() << "event";

    if (m_initialized) {

    }

    return QWindow::event(event);
}

void VulkanWindow::resizeEvent(QResizeEvent* event)
{
    emit debugLogSent("resized");
    qDebug() << "resized";

    if (m_initialized) {

    }

    QWindow::resizeEvent(event);
}
