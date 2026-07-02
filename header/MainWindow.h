#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "VulkanWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow() override;

private:
    void initialize();
    void initializeLoggers();
    void initializeVulkanWindow();

private slots:
    void printVulkanLog(const QString& iLog);
    void printDebugLog(const QString& iLog);

private:
    static constexpr int MAX_LOG_LINES{200};

private:
    std::unique_ptr<Ui::MainWindow> m_ui{nullptr};

    // Vulkan Window
    std::unique_ptr<VulkanWindow> m_vulkanWindow{nullptr};
    static constexpr uint32_t VULKAN_WIDGET_WIDTH{900};
    static constexpr uint32_t VULKAN_WIDGET_HEIGHT{500};

};
#endif // MAINWINDOW_H
