#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include <QVulkanInstance>
#include <QVulkanFunctions>

#include "GraphicDevice.h"
#include "SwapChain.h"

#include <glm/glm.hpp>


typedef struct Vertex {
    glm::vec3 pos; // Vertex position (x, y, z)
    glm::vec3 col; // Vertex color
    glm::vec2 uv;
} vertex_t;


class VulkanWindow;

class VulkanRenderer
{
public:
    explicit VulkanRenderer(VulkanWindow* iWindow) noexcept;
    virtual ~VulkanRenderer();

    VulkanRenderer(const VulkanRenderer& iOther) = delete;
    VulkanRenderer& operator=(const VulkanRenderer& iOther) = delete;

    VulkanRenderer(VulkanRenderer&& iOther) = delete;
    VulkanRenderer& operator=(VulkanRenderer&& iOther) = delete;

    bool initializeResources();
    void cleanup();

    void recreateImageDependentResources();

private:
    // LOGGING
    void printVulkanLog(const QString& iString) const;
    void printDebugLog(const QString& iString) const;

private:
    void createSurface();
    void createGraphicDevice();
    void createSwapChain();
    void createDescriptorSetLayout();
    void createPushConstantRange();
    void createGraphicsPipeline();

private:
    // Vulkan Window
    VulkanWindow* m_pWindow{nullptr};

    // Surface
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};
    VkSurfaceFormatKHR m_surfaceFormat{};

    // Graphic Device
    std::unique_ptr<GraphicDevice> m_graphicDevice{nullptr};

    // SwapChain
    std::unique_ptr<SwapChain> m_swapChain{nullptr};

    // - Descriptor Set Layout
    VkDescriptorSetLayout m_descriptorSetLayout{VK_NULL_HANDLE};

    // - Descriptor sets
    std::vector<VkDescriptorSet> m_descriptorSets;

    // - Descriptor pool
    VkDescriptorPool m_descriptorPool{VK_NULL_HANDLE};

    // Push Constants
    VkPushConstantRange m_pushConstantRange{};

    // Graphics Pipeline
    VkPipelineLayout m_graphicPipelineLayout{VK_NULL_HANDLE};
    VkPipeline m_graphicPipeline{VK_NULL_HANDLE};
};

#endif // VULKANRENDERER_H
