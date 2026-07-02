#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QtAssert>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_ui(std::make_unique<Ui::MainWindow>())
    , m_vulkanWindow(std::make_unique<VulkanWindow>())
{
    m_ui->setupUi(this);

    initialize();
}

MainWindow::~MainWindow()
{
    m_ui = nullptr;
    m_vulkanWindow = nullptr;
}

void MainWindow::initialize()
{
    initializeLoggers();
    initializeVulkanWindow();
}

void MainWindow::initializeLoggers()
{
    // CONNECT
    connect(m_vulkanWindow.get(), &VulkanWindow::vulkanLogSent, this, &MainWindow::printVulkanLog);
    connect(m_vulkanWindow.get(), &VulkanWindow::debugLogSent, this, &MainWindow::printDebugLog);

    m_ui->vulkanLogger->setMaximumBlockCount(MAX_LOG_LINES);
    m_ui->debugLogger->setMaximumBlockCount(MAX_LOG_LINES);
}

void MainWindow::initializeVulkanWindow()
{
    Q_ASSERT(m_vulkanWindow);

    // Vulkan instance should be created before to call QWidget::createWindowContainer() method
    if (!m_vulkanWindow->createVulkanInstance()) {
        qCritical() << "Failed to create vulkan instance";
    }

    // Window Container
    QWidget* pWindowContainer = QWidget::createWindowContainer(m_vulkanWindow.get(), m_ui->vulkanWindow->parentWidget());
    pWindowContainer->setSizePolicy(m_ui->vulkanWindow->sizePolicy());
    // pWindowContainer->setMinimumSize(m_ui->vulkanWindow->minimumSize());
    //pWindowContainer->resize(VULKAN_WIDGET_WIDTH, VULKAN_WIDGET_HEIGHT);


    // Replace Vulkan Window
    {
        auto* replacedItem = m_ui->vulkanLayout->replaceWidget(m_ui->vulkanWindow, pWindowContainer);
        delete replacedItem;
        delete m_ui->vulkanWindow;
        m_ui->vulkanWindow = pWindowContainer;
    }
}


void MainWindow::printVulkanLog(const QString& iLog)
{
    m_ui->vulkanLogger->appendPlainText(iLog);
}

void MainWindow::printDebugLog(const QString& iLog)
{
    m_ui->debugLogger->appendPlainText(iLog);
}