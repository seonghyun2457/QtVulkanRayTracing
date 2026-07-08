#ifndef GRAPHICDEVICE_H
#define GRAPHICDEVICE_H

#include <QVulkanInstance>

#include <vector>

class VulkanWindow;

typedef struct {
    uint32_t graphics;
    uint32_t compute;
    uint32_t transfer;
} queueFamilyIndices_t;

class GraphicDevice
{
public:
    explicit GraphicDevice(VulkanWindow* ipWindow, const VkSurfaceKHR& iSurface) noexcept;
    virtual ~GraphicDevice();

    GraphicDevice(const GraphicDevice& iOther) = delete;
    GraphicDevice& operator=(const GraphicDevice& iOther) = delete;

    GraphicDevice(GraphicDevice&& iOther) = delete;
    GraphicDevice& operator=(GraphicDevice&& iOther) = delete;

    void createGraphicDevice();
    void destroy();

    inline const VkPhysicalDevice& getPhysicalDevice() const { return m_physicalDevice; }
    inline const VkDevice& getDevice() const { return m_logicalDevice; }
    inline const queueFamilyIndices_t& getQueueFamilyIndices() const { return m_queueFaimilyIndices; }

private:
    void selectPhysicalDevice();
    void createLogicalDevice();
    void createQueues();


    const uint32_t getQueueFamilyIndex(const VkQueueFlags iQueueFlags) const;
    bool isSupportedExtension(const char* iExtension) const;

private:
    // LOGGING
    void printVulkanLog(const QString& iString) const;
    void printDebugLog(const QString& iString) const;

private:
    // VulkanWindow
    VulkanWindow* m_pWindow{nullptr};

    // Surface
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};


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
    queueFamilyIndices_t m_queueFaimilyIndices{ uint32_t(-1), uint32_t(-1), uint32_t(-1) };

    // - Queues
    VkQueue m_graphicsQueue{VK_NULL_HANDLE};
    VkQueue m_computeQueue{VK_NULL_HANDLE};
    VkQueue m_transferQueue{VK_NULL_HANDLE};
};

#endif // GRAPHICDEVICE_H
