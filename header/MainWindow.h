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


private:
    static constexpr int MAX_LOG_LINES{200};

private:
    std::unique_ptr<Ui::MainWindow> m_ui{nullptr};
    std::unique_ptr<VulkanWindow> m_vulkanWindow{nullptr};
};
#endif // MAINWINDOW_H
