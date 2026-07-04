#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QtAssert>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_ui(std::make_unique<Ui::MainWindow>())
    , m_vulkanWindow(new VulkanWindow())
{
    m_ui->setupUi(this);

    initialize();
}

MainWindow::~MainWindow()
{
    m_vulkanWindow = nullptr;
    m_ui = nullptr;
}

void MainWindow::initialize()
{
    initializeLoggers();
    initializeVulkanWindow();
}

void MainWindow::initializeLoggers()
{
    // CONNECT
    connect(m_vulkanWindow, &VulkanWindow::vulkanLogSent, this, &MainWindow::printVulkanLog);
    connect(m_vulkanWindow, &VulkanWindow::debugLogSent, this, &MainWindow::printDebugLog);

    m_ui->vulkanLogger->setMaximumBlockCount(MAX_LOG_LINES);
    m_ui->debugLogger->setMaximumBlockCount(MAX_LOG_LINES);
}

void MainWindow::initializeVulkanWindow()
{
    Q_ASSERT(m_vulkanWindow);

    createVulkanInstance();

    m_vulkanWindow->setVulkanInstance(&m_vulkanInstance);

    // Window Container
    QWidget* pWindowContainer = QWidget::createWindowContainer(m_vulkanWindow, m_ui->vulkanWindow->parentWidget());
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

void MainWindow::createVulkanInstance()
{
    printDebugLog("Create vulkan Instance");

    m_vulkanInstance.setApiVersion(m_vulkanInstance.supportedApiVersion());
    m_vulkanInstance.setLayers({ "VK_LAYER_KHRONOS_validation" });

    if (!m_vulkanInstance.create()) {
        qCritical() << "Failed to create QVulkanInstance, error code:" << m_vulkanInstance.errorCode();
        exit(1);
    }

    printVulkanLog("Extensions:");
    for (const auto& extension : m_vulkanInstance.extensions()) {
        printVulkanLog(extension);
    }

    printVulkanLog("Vulkan api version: " + m_vulkanInstance.apiVersion().toString());
}

void MainWindow::printVulkanLog(const QString& iLog)
{
    m_ui->vulkanLogger->appendPlainText(iLog);
}

void MainWindow::printDebugLog(const QString& iLog)
{
    m_ui->debugLogger->appendPlainText(iLog);
}