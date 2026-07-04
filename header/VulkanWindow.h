#ifndef VULKANWINDOW_H
#define VULKANWINDOW_H

#include <QWindow>

#include "VulkanRenderer.h"

class VulkanWindow : public QWindow
{
    Q_OBJECT
public:
    VulkanWindow();
    virtual ~VulkanWindow() override;

    void initializeRenderer();

signals:
    void vulkanLogSent(const QString& iLog);
    void debugLogSent(const QString& iLog);

protected:
    virtual void exposeEvent(QExposeEvent* event) override;
    virtual bool event(QEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;

private:
    std::unique_ptr<VulkanRenderer> m_vulkanRenderer{nullptr};
    bool m_initialized{false};
};

#endif // VULKANWINDOW_H
