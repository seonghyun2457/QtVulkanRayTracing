#include "VulkanWindow.h"

#include <QDebug>
#include <QExposeEvent>
#include <QOffscreenSurface>
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
    if (event->type() == QEvent::PlatformSurface) {
        auto* surfaceEvent = static_cast<QPlatformSurfaceEvent*>(event);

        if (surfaceEvent->surfaceEventType() == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed) {
            if (m_vulkanRenderer) {
                qDebug() << "Surface to be destroyed";

                m_initialized = false;
                m_vulkanRenderer->cleanup();
            }
        }
    }

    return QWindow::event(event);
}

void VulkanWindow::resizeEvent(QResizeEvent* event)
{
    emit debugLogSent("resized");
    qDebug() << "resized";

    if (m_initialized) {
        m_vulkanRenderer->recreateImageDependentResources();
    }

    QWindow::resizeEvent(event);
}
