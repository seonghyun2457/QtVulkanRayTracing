#ifndef GRAPHICDEVICE_H
#define GRAPHICDEVICE_H

#include <QVulkanInstance>

#include <vector>

class VulkanWindow;

class GraphicDevice
{
public:
    explicit GraphicDevice(VulkanWindow* iWindow) noexcept;
    virtual ~GraphicDevice();

    GraphicDevice(const GraphicDevice& iOther) = delete;
    GraphicDevice& operator=(const GraphicDevice& iOther) = delete;

    GraphicDevice(GraphicDevice&& iOther) = delete;
    GraphicDevice& operator=(GraphicDevice&& iOther) = delete;

    void createGraphicDevice(QVulkanInstance& iVulkanInstance, const VkSurfaceKHR& iSurface);
    void destroy(QVulkanInstance& iVulkanInstance);

private:
    void selectPhysicalDevice(QVulkanInstance& iVulkanInstance, const VkSurfaceKHR& iSurface);
    void createLogicalDevice(QVulkanInstance& iVulkanInstance);
    void createQueues(QVulkanInstance& iVulkanInstance);


    const uint32_t getQueueFamilyIndex(const VkQueueFlags iQueueFlags) const;
    bool isSupportedExtension(const char* iExtension) const;

private:
    // LOGGING
    void printVulkanLog(const QString& iString) const;
    void printDebugLog(const QString& iString) const;

private:
    // VulkanWindow
    VulkanWindow* m_pWindow{nullptr};

    // - Devices
    // -- Physical Deivce
    VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
    VkPhysicalDeviceProperties m_physicalDeviceProperties{};
    VkPhysicalDeviceFeatures m_physicalDeviceFeatures{};
    VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties{};

    // -- Logical Deivce
    VkDevice m_logicalDevice{VK_NULL_HANDLE};

    // - Extensions
    std::vector<std::string> m_supportedExtensions;

    // - Queue Familiy
    std::vector<VkQueueFamilyProperties> m_queueFamilyProperties;
    struct {
        uint32_t graphics = uint32_t(-1);
        uint32_t compute = uint32_t(-1);
        uint32_t transfer = uint32_t(-1);
    } m_queueFaimilyIndices{};

    // - Queues
    VkQueue m_graphicsQueue{VK_NULL_HANDLE};
    VkQueue m_computeQueue{VK_NULL_HANDLE};
    VkQueue m_transferQueue{VK_NULL_HANDLE};


private:
    // SUPPORT
    // - Pointer to functions
    // QVulkanFunctions* m_pFunctions{VK_NULL_HANDLE};
    // QVulkanDeviceFunctions* m_pDeviceFunctions{VK_NULL_HANDLE};
};

#endif // GRAPHICDEVICE_H
