#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include <QVulkanInstance>
#include <QVulkanFunctions>

#include "SwapChain.h"

#include <glm/glm.hpp>

#include <vector>


typedef struct Vertex {
    glm::vec3 pos; // Vertex position (x, y, z)
    glm::vec3 col; // Vertex color
    glm::vec2 uv;
} vertex_t;


class VulkanWindow;
class GraphicDevice;
class Buffer;

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

    void draw();

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
    void createQueryPools();
    void createCommandPools();
    VkCommandPool createCommandPool(const uint32_t iQueueFamilyIndex);
    void createCommandBuffers();
    void createUniformBuffers();
    void destroyUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    void createSynchronization();

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

    // Descriptor Set Layout
    VkDescriptorSetLayout m_descriptorSetLayout{VK_NULL_HANDLE};

    // Descriptor sets
    std::vector<VkDescriptorSet> m_descriptorSets;

    // - Descriptor pool
    VkDescriptorPool m_descriptorPool{VK_NULL_HANDLE};

    // Push Constants
    VkPushConstantRange m_pushConstantRange{};

    // Graphics Pipeline
    VkPipelineLayout m_graphicPipelineLayout{VK_NULL_HANDLE};
    VkPipeline m_graphicPipeline{VK_NULL_HANDLE};

    // GPU FPS
    // Query Count
    static constexpr uint32_t FPS_QUERY_COUNT{2}; // Start timestamp and End timestamp

    // -- gpu time in milliseconds
    std::vector<float> m_gpuTimesMs;

    // Query pools
    std::vector<VkQueryPool> m_queryPools;

    // Command pools
    VkCommandPool m_graphicsCommandPool{VK_NULL_HANDLE};
    VkCommandPool m_computeCommandPool{VK_NULL_HANDLE};
    VkCommandPool m_transferCommandPool{VK_NULL_HANDLE};

    // Command buffers
    std::vector<VkCommandBuffer> m_graphicsCommandBuffers;

    // Synchronizaiton resources
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT{2}; // Normally 2 or 3
    std::vector<VkSemaphore> m_imagesAvailable;
    std::vector<VkSemaphore> m_renderFinished;
    std::vector<VkFence> m_fences;
    size_t m_currentFrame{0};
    size_t m_prevFrame{0};

    // Uniform buffers
    std::vector<std::unique_ptr<Buffer>> m_uboMvpBuffers;

    struct UboModelViewProjection {
        glm::mat4 model{0.f};
        glm::mat4 view{0.f};
        glm::mat4 projection{0.f};
    } m_uboModelViewProjection;
};

#endif // VULKANRENDERER_H
