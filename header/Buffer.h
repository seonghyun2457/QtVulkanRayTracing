#ifndef BUFFER_H
#define BUFFER_H

#include <QVulkanInstance>

#include "GraphicDevice.h"

class Buffer
{
public:
    Buffer() = delete;
    explicit Buffer(const GraphicDevice* ipGraphicDevice) noexcept;
    virtual ~Buffer();

    Buffer(const Buffer& iOther) = delete;
    Buffer& operator=(const Buffer& iOther) = delete;

    Buffer(Buffer&& iOther) = delete;
    Buffer& operator=(Buffer&& iOther) = delete;

    void createBuffer(const VkDeviceSize iBufferSize,
                      const VkBufferUsageFlags iBufferUsageFlags,
                      const VkMemoryPropertyFlags iBufferProperties);

    void destroy();

    inline const VkBuffer& getBuffer() const { return m_buffer; };

private:
    void destroyBuffer();
    void destroyBufferMemory();

private:
    const GraphicDevice* m_pGraphicDevice;

    VkBuffer m_buffer{VK_NULL_HANDLE};
    VkDeviceMemory m_bufferMemory{VK_NULL_HANDLE};
};

#endif // BUFFER_H
