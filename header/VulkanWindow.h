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

    void createRenderer();

private:
    std::unique_ptr<VulkanRenderer> m_vulkanRenderer{nullptr};
};

#endif // VULKANWINDOW_H
