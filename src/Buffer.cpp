#include "Buffer.h"

#include <QVulkanDeviceFunctions>

Buffer::Buffer(const GraphicDevice* ipGraphicDevice) noexcept
    : m_pGraphicDevice(ipGraphicDevice)
{

}

Buffer::~Buffer()
{
    destroy();
}

void Buffer::createBuffer(const VkDeviceSize iBufferSize, const VkBufferUsageFlags iBufferUsageFlags, const VkMemoryPropertyFlags iBufferProperties)
{
    QVulkanDeviceFunctions* pDeviceFunctions = m_pGraphicDevice->getVulkanDeviceFunctions();
    Q_ASSERT(pDeviceFunctions != nullptr);

    // Create vertex buffer
    // Buffer creation information
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = iBufferSize;                       // Buffer size
    bufferCreateInfo.usage = iBufferUsageFlags;                // Multiple type of buffer possible
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // Similar to swapchain images, can share vertex buffers

    VkResult result = pDeviceFunctions->vkCreateBuffer(m_pGraphicDevice->getDevice(), &bufferCreateInfo, nullptr, &m_buffer);
    if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Vertex buffer");

    // Get buffer memory requirements
    VkMemoryRequirements memRequirements{};
    pDeviceFunctions->vkGetBufferMemoryRequirements(m_pGraphicDevice->getDevice(), m_buffer, &memRequirements);

    // Malloc information
    VkMemoryAllocateInfo mallocInfo{};
    mallocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mallocInfo.allocationSize = memRequirements.size;
    mallocInfo.memoryTypeIndex = m_pGraphicDevice->findMemoryTypeIndex(memRequirements.memoryTypeBits, iBufferProperties);

    // Allocate memory to buffer
    result= pDeviceFunctions->vkAllocateMemory(m_pGraphicDevice->getDevice(), &mallocInfo, nullptr, &m_bufferMemory);
    if (result != VK_SUCCESS) throw std::runtime_error("Failed to allocate Vertex buffer memroy");

    // Allocate memory to given vertex buffer
    pDeviceFunctions->vkBindBufferMemory(m_pGraphicDevice->getDevice(), m_buffer, m_bufferMemory, 0);
}

void Buffer::destroy()
{
    destroyBuffer();
    destroyBufferMemory();
}

void Buffer::destroyBuffer()
{
    QVulkanDeviceFunctions* pDeviceFunctions = m_pGraphicDevice->getVulkanDeviceFunctions();
    Q_ASSERT(pDeviceFunctions != nullptr);

    if (m_buffer) {
        pDeviceFunctions->vkDestroyBuffer(m_pGraphicDevice->getDevice(), m_buffer, nullptr);
        m_buffer = VK_NULL_HANDLE;
    }
}

void Buffer::destroyBufferMemory()
{
    QVulkanDeviceFunctions* pDeviceFunctions = m_pGraphicDevice->getVulkanDeviceFunctions();
    Q_ASSERT(pDeviceFunctions != nullptr);

    if (m_bufferMemory) {
        pDeviceFunctions->vkFreeMemory(m_pGraphicDevice->getDevice(), m_bufferMemory, nullptr);
        m_bufferMemory = VK_NULL_HANDLE;
    }
}