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
    m_ui->vulkanLogger->setMaximumBlockCount(MAX_LOG_LINES);
    m_ui->debugLogger->setMaximumBlockCount(MAX_LOG_LINES);

    m_ui->vulkanLogger->appendPlainText("Hello world");
    m_ui->debugLogger->appendPlainText("I'm Seonghyun");
}

void MainWindow::initializeVulkanWindow()
{
    Q_ASSERT(m_vulkanWindow);

    m_vulkanWindow->createRenderer();
}
