#include "MainWindow.h"

#include <QApplication>


#include <QVulkanInstance>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QVulkanInstance vulkanInstance;
    vulkanInstance.setApiVersion(vulkanInstance.supportedApiVersion());
    vulkanInstance.setLayers({ "VK_LAYER_KHRONOS_validation" });

    if (!vulkanInstance.create()) {
        qCritical() << "Failed to create QVulkanInstance, error code:" << vulkanInstance.errorCode();
        exit(1);
    }

    MainWindow mainWindow(&vulkanInstance);
    mainWindow.show();

    return QApplication::exec();
}
