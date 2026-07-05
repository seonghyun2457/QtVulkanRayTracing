#include "GraphicDevice.h"

// Qt
#include <QVulkanInstance>
#include <QVulkanFunctions>
#include <QVariant>
#include <QString>

//
#include "VulkanWindow.h"

GraphicDevice::GraphicDevice(VulkanWindow* ipWindow, const VkSurfaceKHR& iSurface) noexcept
    : m_pWindow(ipWindow)
    , m_surface(iSurface)
{

}

GraphicDevice::~GraphicDevice()
{
    destroy();
    qDebug() << "Destroyed GraphicDevice";
}

void GraphicDevice::createGraphicDevice()
{
    selectPhysicalDevice();
    createLogicalDevice();
}

void GraphicDevice::destroy()
{
    QVulkanInstance* pVulkanInstance = m_pWindow->vulkanInstance();

    if (pVulkanInstance && pVulkanInstance->isValid() && m_logicalDevice) {
        QVulkanDeviceFunctions* pDeviceFunctions = pVulkanInstance->deviceFunctions(m_logicalDevice);
        Q_ASSERT(pDeviceFunctions != nullptr);

        // Wait until idle status
        pDeviceFunctions->vkDeviceWaitIdle(m_logicalDevice);

        pDeviceFunctions->vkDestroyDevice(m_logicalDevice, nullptr);
        m_logicalDevice = VK_NULL_HANDLE;
    }
}

void GraphicDevice::selectPhysicalDevice()
{
    QVulkanInstance* pVulkanInstance = m_pWindow->vulkanInstance();

    Q_ASSERT(pVulkanInstance && pVulkanInstance->isValid());

    printDebugLog("Select physical device");

    QVulkanFunctions* pFunctions = pVulkanInstance->functions();

    uint32_t deviceCount = 0;
    pFunctions->vkEnumeratePhysicalDevices(pVulkanInstance->vkInstance(), &deviceCount, nullptr);

    // If no device available, then none support Vulkan!
    if (deviceCount == 0) throw std::runtime_error("Can't find GPUs that support Vulkan Instance!");

    // Get list of physical devices
    std::vector<VkPhysicalDevice> gpuDeviceList(deviceCount);
    pFunctions->vkEnumeratePhysicalDevices(pVulkanInstance->vkInstance(), &deviceCount, gpuDeviceList.data());

    // Pick suitable physical device
    // Always select first index device
    uint32_t selectedGPUIndex = 0;
    m_physicalDevice = gpuDeviceList[selectedGPUIndex];

    // Get Properties of our new physical device
    pFunctions->vkGetPhysicalDeviceProperties(m_physicalDevice, &m_physicalDeviceProperties);
    printVulkanLog("GPU name: " + QString(m_physicalDeviceProperties.deviceName));

    // Get Physical device features
    pFunctions->vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_physicalDeviceFeatures);
    printVulkanLog("Physical device features: ");
    printVulkanLog("geometryShader: " + QVariant(m_physicalDeviceFeatures.geometryShader).toString());
    printVulkanLog("tessellationShader: " + QVariant(m_physicalDeviceFeatures.tessellationShader).toString());

    // Get Physical device memory properties
    pFunctions->vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_physicalDeviceMemoryProperties);

    // Find Queue Family
    printDebugLog("Get queue familiy");

    uint32_t queueFamilyCount = 0;
    pFunctions->vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
    Q_ASSERT(queueFamilyCount > 0);

    m_queueFamilyProperties.resize(queueFamilyCount);
    pFunctions->vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, m_queueFamilyProperties.data());

    printVulkanLog("Number of Queue Family Properties: " + QString::number(queueFamilyCount));
    for (size_t i = 0; i < m_queueFamilyProperties.size(); ++i) {
        QString queueFamily = "Queue Familiy " + QString::number(i) + ": " + QString::number(m_queueFamilyProperties[i].queueCount) + ", flags: ";
        if (m_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queueFamily += "VK_QUEUE_GRAPHICS_BIT";
        } else if (m_queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            queueFamily += "VK_QUEUE_COMPUTE_BIT";
        } else if (m_queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            queueFamily += "VK_QUEUE_TRANSFER_BIT";
        } else if (m_queueFamilyProperties[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
            queueFamily += "VK_QUEUE_SPARSE_BINDING_BIT";
        } else if (m_queueFamilyProperties[i].queueFlags & VK_QUEUE_PROTECTED_BIT) {
            queueFamily += "VK_QUEUE_PROTECTED_BIT";
        } else if (m_queueFamilyProperties[i].queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) {
            queueFamily += "VK_QUEUE_VIDEO_DECODE_BIT_KHR";
        } else if (m_queueFamilyProperties[i].queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) {
            queueFamily += "VK_QUEUE_VIDEO_ENCODE_BIT_KHR";
        } else if (m_queueFamilyProperties[i].queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV) {
            queueFamily += "VK_QUEUE_OPTICAL_FLOW_BIT_NV";
        }

        printVulkanLog(queueFamily);
    }

    // Extensions
    {
        uint32_t extensionCount = 0;
        pFunctions->vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, nullptr);
        if (extensionCount > 0) {
            std::vector<VkExtensionProperties> extensions(extensionCount);
            pFunctions->vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, extensions.data());
            for (const VkExtensionProperties& extension : extensions) {
                m_supportedExtensions.push_back(extension.extensionName);
            }
        }
    }

    // Check if Graphics queue supports Presentation queue
    {
        const uint32_t graphicsQueueIndex = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
        VkBool32 presentSupport = VK_FALSE;

        auto* vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)pVulkanInstance->getInstanceProcAddr("vkGetPhysicalDeviceSurfaceSupportKHR");
        Q_ASSERT(vkGetPhysicalDeviceSurfaceSupportKHR != nullptr);
        vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, graphicsQueueIndex, m_surface, &presentSupport);

        if (!presentSupport) throw std::runtime_error("Graphics queue family does not support presentation!");

        printDebugLog("Graphics queue supports presentation: OK (index: " + QString::number(graphicsQueueIndex) + ")");
    }
}

void GraphicDevice::createLogicalDevice()
{
    QVulkanInstance* pVulkanInstance = m_pWindow->vulkanInstance();

    Q_ASSERT(pVulkanInstance && pVulkanInstance->isValid());

    printDebugLog("Create logical device");

    QVulkanFunctions* pFunctions = pVulkanInstance->functions();
    Q_ASSERT(pFunctions != nullptr);

    const VkQueueFlags requestedQueueTypes = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT;

    VkPhysicalDeviceVulkan13Features enabledFeatures13{};
    enabledFeatures13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    enabledFeatures13.dynamicRendering = VK_TRUE; // Enable Dynamic Rendering
    enabledFeatures13.synchronization2 = VK_TRUE;

    VkPhysicalDeviceVulkan12Features enabledFeatures12{};
    enabledFeatures12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    enabledFeatures12.hostQueryReset = VK_TRUE; // Enable CPU to call vkResetQueryPool
    enabledFeatures12.pNext = &enabledFeatures13; // Connect VkPhysicalDeviceVulkan13Features

    // QUEUE FAMILIY
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    const float defaultQueueProiority = 0.f;

    // Graphics queue
    if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {
        m_queueFaimilyIndices.graphics = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);

        VkDeviceQueueCreateInfo graphicQueueInfo{};
        graphicQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        graphicQueueInfo.queueFamilyIndex = m_queueFaimilyIndices.graphics;
        graphicQueueInfo.queueCount = 1;
        graphicQueueInfo.pQueuePriorities = &defaultQueueProiority;

        queueCreateInfos.push_back(graphicQueueInfo);
    } else {
        m_queueFaimilyIndices.graphics = 0;
    }

    // Compute queue
    if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT) {
        m_queueFaimilyIndices.compute = getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
        if (m_queueFaimilyIndices.compute != m_queueFaimilyIndices.graphics) {

            VkDeviceQueueCreateInfo computeQueueInfo{};
            computeQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            computeQueueInfo.queueFamilyIndex = m_queueFaimilyIndices.compute;
            computeQueueInfo.queueCount = 1;
            computeQueueInfo.pQueuePriorities = &defaultQueueProiority;

            queueCreateInfos.push_back(computeQueueInfo);
        }
    } else {
        m_queueFaimilyIndices.compute = m_queueFaimilyIndices.graphics;
    }

    // Transfer queue
    if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT) {
        m_queueFaimilyIndices.transfer = getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);

        if ((m_queueFaimilyIndices.transfer != m_queueFaimilyIndices.graphics) &&
            (m_queueFaimilyIndices.transfer != m_queueFaimilyIndices.compute)) {

            VkDeviceQueueCreateInfo transferQueueInfo{};
            transferQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            transferQueueInfo.queueFamilyIndex = m_queueFaimilyIndices.transfer;
            transferQueueInfo.queueCount = 1;
            transferQueueInfo.pQueuePriorities = &defaultQueueProiority;

            queueCreateInfos.push_back(transferQueueInfo);
        }
    } else {
        m_queueFaimilyIndices.transfer = m_queueFaimilyIndices.graphics;
    }

    // Enable features
    VkPhysicalDeviceFeatures enabledFeatures{};
    enabledFeatures.samplerAnisotropy = m_physicalDeviceFeatures.samplerAnisotropy;
    /*
    enabledFeatures.depthBiasClamp = m_physicalDeviceFeatures.depthBiasClamp;
    enabledFeatures.depthClamp = m_physicalDeviceFeatures.depthClamp;
    */

    // Check if the physical device supports extensions
    std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    for (const char* extension : deviceExtensions) {
        if (!isSupportedExtension(extension)) {
            printDebugLog("Enabled device extension " + QString(extension) + " isn't present at device level.");
        }
    }

    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};
    physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    physicalDeviceFeatures2.features = enabledFeatures;
    physicalDeviceFeatures2.pNext = &enabledFeatures12;

    // Information to create logical device
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = nullptr;
    deviceCreateInfo.pNext = &physicalDeviceFeatures2;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    // Create the logical device for the physical device
    VkResult result =  pFunctions->vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_logicalDevice);
    if (result != VK_SUCCESS) throw std::runtime_error("Failed to create a logical device");

    // Queues are created at the same time as the logical device
    createQueues();
}

void GraphicDevice::createQueues()
{
    QVulkanInstance* pVulkanInstance = m_pWindow->vulkanInstance();

    Q_ASSERT(pVulkanInstance && pVulkanInstance->isValid());
    printDebugLog("Create Queues");

    QVulkanDeviceFunctions* pDeviceFunctions = pVulkanInstance->deviceFunctions(m_logicalDevice);
    Q_ASSERT(pDeviceFunctions != nullptr);

    // Validate queue family indices before getting queues
    if (m_queueFaimilyIndices.graphics == -1) {
        throw std::runtime_error("Graphics queue family index is invalid");
    }

    // From given logical device, of given Queue Family, of given Queue Index (0 since only one queue), place reference in given VkQueue
    pDeviceFunctions->vkGetDeviceQueue(m_logicalDevice, m_queueFaimilyIndices.graphics, 0, &m_graphicsQueue);

    // For compute queue: either get from dedicated family or use graphics queue
    if ((m_queueFaimilyIndices.compute != m_queueFaimilyIndices.graphics) && (m_queueFaimilyIndices.compute != -1)) {
        pDeviceFunctions->vkGetDeviceQueue(m_logicalDevice, m_queueFaimilyIndices.compute, 0, &m_computeQueue);
    } else {
        m_computeQueue = m_graphicsQueue;
    }

    // For transfer queue: either get from dedicated family or use appropriate fallback
    if ((m_queueFaimilyIndices.transfer != m_queueFaimilyIndices.graphics) && (m_queueFaimilyIndices.transfer != m_queueFaimilyIndices.compute) && (m_queueFaimilyIndices.transfer != -1)) {
        pDeviceFunctions->vkGetDeviceQueue(m_logicalDevice, m_queueFaimilyIndices.transfer, 0, &m_transferQueue);
    } else if (m_queueFaimilyIndices.transfer == m_queueFaimilyIndices.compute) {
        m_transferQueue = m_computeQueue;
    } else {
        m_transferQueue = m_graphicsQueue;
    }

    // Validate all queues were obtained
    if (m_graphicsQueue == VK_NULL_HANDLE) throw std::runtime_error("Failed to get Graphic queue");
    if (m_computeQueue == VK_NULL_HANDLE) throw std::runtime_error("Failed to get Compute queue");
    if (m_transferQueue == VK_NULL_HANDLE) throw std::runtime_error("Failed to get Transfer queue");
}

bool GraphicDevice::isSupportedExtension(const char *iExtension) const
{
    return (std::find(m_supportedExtensions.begin(), m_supportedExtensions.end(), iExtension) != m_supportedExtensions.end());
}

const uint32_t GraphicDevice::getQueueFamilyIndex(const VkQueueFlags iQueueFlags) const
{
    if ((iQueueFlags & VK_QUEUE_COMPUTE_BIT) == iQueueFlags) {
        for (size_t i = 0; i < m_queueFamilyProperties.size(); ++i) {
            if ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) &&
                ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)) {
                return i;
            }
        }
    }

    if ((iQueueFlags & VK_QUEUE_TRANSFER_BIT) == iQueueFlags) {
        for (size_t i = 0; i < m_queueFamilyProperties.size(); ++i) {
            if ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
                ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
                ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)) {
                return i;
            }
        }
    }

    for (size_t i = 0; i < m_queueFamilyProperties.size(); ++i) {
        if ((m_queueFamilyProperties[i].queueFlags & iQueueFlags) == iQueueFlags) {
            return i;
        }
    }

    printDebugLog("Couldn't find a queue family that supports the requested queue flags: " + QString::number(iQueueFlags));

    return -1;
}

void GraphicDevice::printVulkanLog(const QString &iString) const
{
    emit m_pWindow->vulkanLogSent(iString);
}

void GraphicDevice::printDebugLog(const QString &iString) const
{
    emit m_pWindow->debugLogSent(iString);
}