#include "VulkanRenderer.h"

#include "VulkanWindow.h"
#include "GraphicDevice.h"
#include "Buffer.h"
#include "Shader.h"

#include <QDebug>
#include <QVariant>

#include <exception>
#include <set>

VulkanRenderer::VulkanRenderer(VulkanWindow* iWindow) noexcept
    : m_pWindow(iWindow)
    , m_graphicDevice(nullptr)
    , m_swapChain(nullptr)
{

}

VulkanRenderer::~VulkanRenderer()
{
    cleanup();
    qDebug() << "Destroyed VulkanRenderer";
}

bool VulkanRenderer::initializeResources()
{
    bool initialized = true;
    try
    {
        createSurface();
        createGraphicDevice();
        createSwapChain();
        createDescriptorSetLayout();
        createPushConstantRange();
        createGraphicsPipeline();

        // Create resources after graphic pipeline
        createQueryPools();
        createCommandPools();
        createCommandBuffers();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createSynchronization();

    } catch (const std::runtime_error& e) {
        printDebugLog(e.what());
        initialized = false;
    }

    return initialized;
}

void VulkanRenderer::cleanup()
{
    QVulkanInstance* pVulkanInstance = m_pWindow->vulkanInstance();

    if (pVulkanInstance && pVulkanInstance->isValid() && m_graphicDevice && m_graphicDevice->getDevice()) {
        QVulkanDeviceFunctions* pDeviceFunctions = m_graphicDevice->getVulkanDeviceFunctions();
        Q_ASSERT(pDeviceFunctions != nullptr);

        // Wait until Idle status
        pDeviceFunctions->vkDeviceWaitIdle(m_graphicDevice->getDevice());

        // Destroy Descriptor pool
        destroyDescriptorPool();


        // Destroy uniform buffers
        destroyUniformBuffers();

        // Destroy Command buffers
        {
            if (!m_graphicsCommandBuffers.empty()) {
                pDeviceFunctions->vkFreeCommandBuffers(m_graphicDevice->getDevice(), m_graphicsCommandPool, static_cast<uint32_t>(m_graphicsCommandBuffers.size()), m_graphicsCommandBuffers.data());
                m_graphicsCommandBuffers.clear();
            }
        }

        // Destroy Command pools
        {
            std::set<VkCommandPool> uniqueCommandPools;

            if (m_graphicsCommandPool != VK_NULL_HANDLE) uniqueCommandPools.insert(m_graphicsCommandPool);
            if (m_computeCommandPool != VK_NULL_HANDLE) uniqueCommandPools.insert(m_computeCommandPool);
            if (m_transferCommandPool != VK_NULL_HANDLE) uniqueCommandPools.insert(m_transferCommandPool);

            for (VkCommandPool commandPool : uniqueCommandPools) {
                pDeviceFunctions->vkDestroyCommandPool(m_graphicDevice->getDevice(), commandPool, nullptr);
            }

            m_graphicsCommandPool = VK_NULL_HANDLE;
            m_computeCommandPool = VK_NULL_HANDLE;
            m_transferCommandPool = VK_NULL_HANDLE;

        }

        // Destroy Query pools
        {
            for (size_t i = 0; i < m_queryPools.size(); ++i) {
                if (m_queryPools[i] != VK_NULL_HANDLE) {
                    pDeviceFunctions->vkDestroyQueryPool(m_graphicDevice->getDevice(), m_queryPools[i], nullptr);
                    m_queryPools[i] = VK_NULL_HANDLE;
                }
            }
        }


        // Destroy Graphics pipeline and layout
        {
            auto* vkDestroyPipeline = (PFN_vkDestroyPipeline)(pVulkanInstance->getInstanceProcAddr("vkDestroyPipeline"));
            Q_ASSERT(vkDestroyPipeline != nullptr);
            if (m_graphicPipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(m_graphicDevice->getDevice(), m_graphicPipeline, nullptr);
                m_graphicPipeline = VK_NULL_HANDLE;
            }

            auto* vkDestroyPipelineLayout = (PFN_vkDestroyPipelineLayout)(pVulkanInstance->getInstanceProcAddr("vkDestroyPipelineLayout"));
            Q_ASSERT(vkDestroyPipelineLayout != nullptr);
            if (m_graphicPipelineLayout != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(m_graphicDevice->getDevice(), m_graphicPipelineLayout, nullptr);
                m_graphicPipelineLayout = VK_NULL_HANDLE;
            }
        }

        // Destroy descriptor set layout
        {
            auto* vkDestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout)(pVulkanInstance->getInstanceProcAddr("vkDestroyDescriptorSetLayout"));
            Q_ASSERT(vkDestroyDescriptorSetLayout != nullptr);
            if (m_descriptorSetLayout != VK_NULL_HANDLE) {
                vkDestroyDescriptorSetLayout(m_graphicDevice->getDevice(), m_descriptorSetLayout, nullptr);
                m_descriptorSetLayout = VK_NULL_HANDLE;
            }
        }

        // Desctroy synchronization resources
        {
            auto* vkDestroySemaphore = (PFN_vkDestroySemaphore)(pVulkanInstance->getInstanceProcAddr("vkDestroySemaphore"));
            Q_ASSERT(vkDestroySemaphore != nullptr);

            auto* vkDestroyFence = (PFN_vkDestroyFence)(pVulkanInstance->getInstanceProcAddr("vkDestroyFence"));
            Q_ASSERT(vkDestroyFence != nullptr);

            for (size_t i = 0; i < m_imagesAvailable.size(); ++i) {
                vkDestroySemaphore(m_graphicDevice->getDevice(), m_imagesAvailable[i], nullptr);
            }

            for (size_t i = 0; i < m_renderFinished.size(); ++i) {
                vkDestroySemaphore(m_graphicDevice->getDevice(), m_renderFinished[i], nullptr);
            }

            for (size_t i = 0; i < m_fences.size(); ++i) {
                vkDestroyFence(m_graphicDevice->getDevice(), m_fences[i], nullptr);
            }
        }

    } else {
        qDebug() << "Vulkan instance doesn't exist";
    }


    // Destroy SwapChain
    if (m_swapChain) {
        m_swapChain = nullptr;
    }

    // Destroy Graphic Device
    if (m_graphicDevice) {
        m_graphicDevice = nullptr;
    };

    qDebug() << "Cleaned up";
}

void VulkanRenderer::recreateImageDependentResources()
{
    printDebugLog("Recreate image dependent resources");

    QVulkanDeviceFunctions* pDeviceFunctions = m_graphicDevice->getVulkanDeviceFunctions();
    Q_ASSERT(pDeviceFunctions != nullptr);

    // Wait until idle status
    pDeviceFunctions->vkDeviceWaitIdle(m_graphicDevice->getDevice());

    const size_t prevSwapChainImageCount = m_swapChain->getSwapchainImageCount();
    m_swapChain->recreateSwapchain(m_surface, m_surfaceFormat);

    if (prevSwapChainImageCount != m_swapChain->getSwapchainImageCount()) {
        // Destroy resources
        {
            // Destroy RenderFinished semaphores
            for (size_t i = 0; i < m_renderFinished.size(); ++i) {
                if (m_renderFinished[i] != VK_NULL_HANDLE) {
                    pDeviceFunctions->vkDestroySemaphore(m_graphicDevice->getDevice(), m_renderFinished[i], nullptr);
                    m_renderFinished[i] = VK_NULL_HANDLE;
                }
            }

            // Destroy Command buffers
            pDeviceFunctions->vkResetCommandPool(m_graphicDevice->getDevice(), m_graphicsCommandPool, 0);

            // Destroy Descriptor pool
            destroyDescriptorPool();

            // Destroy Uniform buffers;
            destroyUniformBuffers();
        }

        // Recreate resources
        {
            createUniformBuffers();
            createDescriptorPool();
            createDescriptorSets();
            createCommandBuffers();
            createRenderFinishedSemaphores();
        }
    }
}

void VulkanRenderer::draw()
{

}

void VulkanRenderer::printVulkanLog(const QString& iString) const
{
    emit m_pWindow->vulkanLogSent(iString);
}

void VulkanRenderer::printDebugLog(const QString& iString) const
{
    emit m_pWindow->debugLogSent(iString);
}

void VulkanRenderer::createSurface()
{
    printDebugLog("Create Surface");

    // Get VkSurfaceKHR info from QWindow
    QVulkanInstance* pVulkanInstance = m_pWindow->vulkanInstance();

    if (pVulkanInstance == nullptr) throw std::runtime_error("Vulkan Instance isn't set in Window.");

    m_surface = pVulkanInstance->surfaceForWindow(m_pWindow);
    if (m_surface == nullptr) throw std::runtime_error("Failed to create surface.");
}

void VulkanRenderer::createGraphicDevice()
{
    Q_ASSERT(m_pWindow);
    Q_ASSERT(m_surface);

    m_graphicDevice = std::make_unique<GraphicDevice>(m_pWindow, m_surface);

    if (m_graphicDevice == nullptr) throw std::runtime_error("Failed to create GraphicDevice instance");

    m_graphicDevice->createGraphicDevice();
}



void VulkanRenderer::createSwapChain()
{
    Q_ASSERT(m_pWindow);
    Q_ASSERT(m_surface);

    m_swapChain = std::make_unique<SwapChain>(m_pWindow, m_graphicDevice.get());

    m_swapChain->createSwapchain(m_surface, m_surfaceFormat);
}

void VulkanRenderer::createDescriptorSetLayout()
{
    printDebugLog("Create Descriptor set layout");

    QVulkanDeviceFunctions* pDeviceFunctions = m_graphicDevice->getVulkanDeviceFunctions();
    Q_ASSERT(pDeviceFunctions != nullptr);

    // CREATE UNIFORM BUFFER DESCRIPTOR SET LAYOUT
    // uboModelViewProjection binding info
    VkDescriptorSetLayoutBinding uboModelViewProjectionLayoutBinding{};
    uboModelViewProjectionLayoutBinding.binding = 0;                                         // Binding point in shader (designated by binding number in shader)
    uboModelViewProjectionLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;  // Type of descriptor (uniform buffer, image sampler, etc)
    uboModelViewProjectionLayoutBinding.descriptorCount = 1;								 // Number of descriptors for binding, only 1 in this case, but could be more with arrays of descriptors in shader
    uboModelViewProjectionLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;			 // Shader stage to bind descriptor to (vertex shader in this case since uboViewProjection matrix is used in vertex shader)
    uboModelViewProjectionLayoutBinding.pImmutableSamplers = nullptr;						 // For image sampling, can specify if sampler is immutable (can't be changed) and give a sampler, but not used in this case

    // std::vector<VkDescriptorSetLayoutBinding> layoutBindings = { uboViewProjectionLayoutBinding, uboModelLayoutBinding };
    std::array<VkDescriptorSetLayoutBinding, 1> layoutBindings = { uboModelViewProjectionLayoutBinding };

    // Create Descriptor Set Layout with given bindings
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size()); // Number of bindings in the layout
    descriptorSetLayoutCreateInfo.pBindings = layoutBindings.data();                           // List of bindings (only one in this case)
    descriptorSetLayoutCreateInfo.flags = 0;                                                   // Optional flags for the layout (e.g. update after bind pool)
    descriptorSetLayoutCreateInfo.pNext = nullptr;                                             // Optional pointer to extend functionality of layout creation (e.g. for push descriptors)

    // Create Descriptor Set Layout
    VkResult result = pDeviceFunctions->vkCreateDescriptorSetLayout(m_graphicDevice->getDevice(), &descriptorSetLayoutCreateInfo, nullptr, &m_descriptorSetLayout);
    if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Descriptor Set Layout");
}

void VulkanRenderer::createPushConstantRange()
{
    printDebugLog("Create Pushconstant Range");

    m_pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // Send PushConstant to fragment shader
    m_pushConstantRange.offset = 0;
    m_pushConstantRange.size = 0; // should define
}

void VulkanRenderer::createGraphicsPipeline()
{
    printDebugLog("Create Graphic pipeline");

    QVulkanDeviceFunctions* pDeviceFunctions = m_graphicDevice->getVulkanDeviceFunctions();
    Q_ASSERT(pDeviceFunctions != nullptr);

    // Graphics pipeline creation info requres the array of shader stage creation info
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    // HANDLE SHADERS
    // Build shader modules to link to Graphics pipeline
    VkShaderModule vertexShaderModule = Shader::createShaderModule(m_graphicDevice.get(), "../../shaders/vert.spv");
    VkShaderModule fragmentShaderModule = Shader::createShaderModule(m_graphicDevice.get(), "../../shaders/frag.spv");

    // Shader stage creation information
    // Vertex stage creation information
    VkPipelineShaderStageCreateInfo vertexShaderCreateInfo{};
    vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;           // Shader stage name
    vertexShaderCreateInfo.module = vertexShaderModule;                  // Shader module to be used by stage
    vertexShaderCreateInfo.pName = "main";                               // Entry point to shader

    // Fragment stage creation information
    VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo{};
    fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;       // Shader stage name
    fragmentShaderCreateInfo.module = fragmentShaderModule;              // Shader module to be used by stage
    fragmentShaderCreateInfo.pName = "main";                             // Entry point to shader

    // Put shader stage creation info to vector
    shaderStages.push_back(vertexShaderCreateInfo);
    shaderStages.push_back(fragmentShaderCreateInfo);

    // CREATE GRAPHICS PIPELINE
    {
        // How the data for a single vertex (including info such as position, color, texture coords, normals, etc) is as a whole
        std::array<VkVertexInputBindingDescription, 1> vertexbindingDescptions{};

        {
            // Binding 0: shared unit-quad (per-vertex)
            vertexbindingDescptions[0].binding = 0;
            vertexbindingDescptions[0].stride = sizeof(Vertex);
            vertexbindingDescptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // How to move between data after each vertex
        }

        // How the data for an attribute is defined within a vertex
        std::array<VkVertexInputAttributeDescription, 3> vertexInputAttributesDescripotions{};

        {
            // Position attribute
            // layout(location = 0) in vec3 pos in Vertex Shader
            vertexInputAttributesDescripotions[0].binding = 0;                         // Which binding the data is at (should be same as above)
            vertexInputAttributesDescripotions[0].location = 0;                        // Location in shader where data will be read from
            vertexInputAttributesDescripotions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // Formate the data will take (also helps to define the size of data). 12 bytes.
            vertexInputAttributesDescripotions[0].offset = offsetof(Vertex, pos);      // Where this attribute is defined in the data for a single vertex

            // Color
            // layout(location = 1) in vec3 color in Vertex Shader
            vertexInputAttributesDescripotions[1].binding = 0;
            vertexInputAttributesDescripotions[1].location = 1;
            vertexInputAttributesDescripotions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            vertexInputAttributesDescripotions[1].offset = offsetof(Vertex, col);

            // UV attribute
            // layout(location = 2) in vec2 uv in Vertex Shader
            vertexInputAttributesDescripotions[2].binding = 0;
            vertexInputAttributesDescripotions[2].location = 2;
            vertexInputAttributesDescripotions[2].format = VK_FORMAT_R32G32_SFLOAT; // 8 Bytes for UV
            vertexInputAttributesDescripotions[2].offset = offsetof(Vertex, uv);
        }


        // VERTEX INPUT STATE
        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
        vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexbindingDescptions.size());
        vertexInputStateCreateInfo.pVertexBindingDescriptions = vertexbindingDescptions.data();
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributesDescripotions.size());
        vertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributesDescripotions.data();

        // INPUT ASSEMBLY
        VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
        pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;    // Default primitive type to assemble vertices as
        pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;                 // Allow overriding of "strip" topology to start new primitives

        VkExtent2D extent = m_swapChain->getExtent();

        // VIEWPORT AND SCISSOR
        // Create a viewport info struct
        VkViewport viewPort{};
        viewPort.x = 0.f;                                       // x start coordinate
        viewPort.y = 0.f;                                       // y start coordinate
        viewPort.width = static_cast<float>(extent.width);    // Width of viewport
        viewPort.height = static_cast<float>(extent.height);  // Height of viewport
        viewPort.minDepth = 0.f;                                // Min framebuffer depth
        viewPort.maxDepth = 1.f;                                // Max framebuffer depth

        // Create a scissor info struct
        VkRect2D scissor{};
        scissor.offset = {0, 0};    // Offset to use region from
        scissor.extent = extent;  // Extent to describe region to use, starting at offset

        VkPipelineViewportStateCreateInfo pipelineViewportSateCreateInfo{};
        pipelineViewportSateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        pipelineViewportSateCreateInfo.viewportCount = 1;
        pipelineViewportSateCreateInfo.pViewports = &viewPort;
        pipelineViewportSateCreateInfo.scissorCount = 1;
        pipelineViewportSateCreateInfo.pScissors = &scissor;

        // DYNAMIC STATES
        // Dynamic states to enable
        std::vector<VkDynamicState> dynamicStates ={VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY};

        // Dynamic state creation information
        VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
        pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        pipelineDynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

        // RASTERIZER
        VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};
        pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;                    // Change if fragments beyond near/far planes are clipped(default) or clamped to plane
        pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;             // Whether to discard data and skip rasterizer. Never creates fragments, Only suitable for pipeline without framebuffer output
        pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;             // how to handle filling points between vertices
        pipelineRasterizationStateCreateInfo.lineWidth = 1.f;                                // How thick lines should be when drawn
        pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;               // Which face of a triangle to cull
        pipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;    // Determine which side is front
        pipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;                     // Whether to add depth bias to fragments (good for stoppoing "shadow acne" in shadow mapping)

        // MULTI-SAMPLING
        VkPipelineMultisampleStateCreateInfo pipelineMultiSampleStateCreateInfo{};
        pipelineMultiSampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        pipelineMultiSampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
        pipelineMultiSampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;


        // BLENDING
        // Blending decides how to blend a new color being written to a fragment, with the old value
        // Color blend attachment State
        VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState{};
        pipelineColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        pipelineColorBlendAttachmentState.blendEnable = VK_TRUE; // Enable blending

        // Blending uses equation: (srcColorBlendFactor * new color) colorBlendOp (dstColorBlendFactor * old color)
        pipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        pipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        pipelineColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
        // Summarised: (VK_BLEND_FACTOR_SRC_ALPHA * new color) + (VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA * old color)
        //             (new color alpha * new color) + ((1 - new color alpha)  * old color)

        pipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        pipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        pipelineColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
        // Summarised: (1 * new alpha) + (0 * old alpha) = new alpha

        // Color blend state create information
        VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
        pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        pipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;                       // Alternative to calculations is to use logical opeations
        pipelineColorBlendStateCreateInfo.attachmentCount = 1;
        pipelineColorBlendStateCreateInfo.pAttachments = &pipelineColorBlendAttachmentState;

        // PIPELINE LAYOUT
        // Pipeline layout create information
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &m_descriptorSetLayout;
        pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
        pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
        // pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        // pipelineLayoutCreateInfo.pPushConstantRanges = &m_pushConstantRange;

        VkResult result = pDeviceFunctions->vkCreatePipelineLayout(m_graphicDevice->getDevice(), &pipelineLayoutCreateInfo, nullptr, &m_graphicPipelineLayout);
        if (result != VK_SUCCESS) throw std::runtime_error("Failed to create pipeline layout");

        // DEPTH STENCIL TESTING
        VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};
        pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;            // Enable depth testing (closer fragments should be rendered in front of farther ones)
        pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;           // Whether to wrtie to depth buffer
        pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;  // Comparison operation to use to compare new depth value with existing one in depth buffer
        pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;     // Depth bounds test: check if depth value exist within a min/max range
        pipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;         // Optional stencil test

        // Rendering create Info for Dynamic rendering
        VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
        pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        pipelineRenderingCreateInfo.pNext = nullptr;
        pipelineRenderingCreateInfo.viewMask = 0;
        pipelineRenderingCreateInfo.colorAttachmentCount = 1;
        VkFormat colorFormat = m_surfaceFormat.format;
        pipelineRenderingCreateInfo.pColorAttachmentFormats = &colorFormat;
        pipelineRenderingCreateInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
        pipelineRenderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;


        // GRAPHICS PIPELINE CREATION
        VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
        graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphicsPipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        graphicsPipelineCreateInfo.pStages = shaderStages.data();
        graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
        graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
        graphicsPipelineCreateInfo.pViewportState = &pipelineViewportSateCreateInfo;
        graphicsPipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
        graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
        graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultiSampleStateCreateInfo;
        graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
        graphicsPipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
        graphicsPipelineCreateInfo.layout = m_graphicPipelineLayout;
        graphicsPipelineCreateInfo.renderPass = VK_NULL_HANDLE;                                    // No need Renderpass here as we use Dynamic rendering
        graphicsPipelineCreateInfo.subpass = 0;
        graphicsPipelineCreateInfo.pNext = &pipelineRenderingCreateInfo;                           // Use Dynamic rendering

        // Pipeline derivatives: Can create multiple pipelines that derive from one another for optimization
        graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;                            // Existing pipeline to derive from
        graphicsPipelineCreateInfo.basePipelineIndex = -1;                                         // or index of pipeline being created to derive from (in case creating multiple at once)

        result = pDeviceFunctions->vkCreateGraphicsPipelines(m_graphicDevice->getDevice(), VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &m_graphicPipeline);
        if (result != VK_SUCCESS) throw std::runtime_error("Failed to create pipeline");
    }

    Shader::destroyShaderModule(m_graphicDevice.get(), fragmentShaderModule);
    Shader::destroyShaderModule(m_graphicDevice.get(), vertexShaderModule);
}

void VulkanRenderer::createQueryPools()
{
    printDebugLog("Create Query pools");

    QVulkanInstance* pVulkanInstance = m_pWindow->vulkanInstance();
    Q_ASSERT(pVulkanInstance && pVulkanInstance->isValid());

    QVulkanDeviceFunctions* pDeviceFunctions = m_graphicDevice->getVulkanDeviceFunctions();
    Q_ASSERT(pDeviceFunctions != nullptr);

    m_queryPools.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
    m_gpuTimesMs.resize(MAX_FRAMES_IN_FLIGHT, 0.f);

    // Query pool creation info for FPS
    VkQueryPoolCreateInfo queryPoolCreateInfo{};
    queryPoolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    queryPoolCreateInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
    queryPoolCreateInfo.queryCount = FPS_QUERY_COUNT; // Start-End

    for (size_t i = 0; i < m_queryPools.size(); ++i) {
        VkResult result = pDeviceFunctions->vkCreateQueryPool(m_graphicDevice->getDevice(), &queryPoolCreateInfo, nullptr, &m_queryPools[i]);
        if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Query pool");

        auto* vkResetQueryPool = (PFN_vkResetQueryPool)pVulkanInstance->getInstanceProcAddr("vkResetQueryPool");
        Q_ASSERT(vkResetQueryPool != nullptr);
        vkResetQueryPool(m_graphicDevice->getDevice(), m_queryPools[i], 0, FPS_QUERY_COUNT);
    }
}

void VulkanRenderer::createCommandPools()
{
    printDebugLog("Create Command Pools");

    const queueFamilyIndices_t queueFamilyIndices = m_graphicDevice->getQueueFamilyIndices();

    // Graphics command pool
    m_graphicsCommandPool = createCommandPool(queueFamilyIndices.graphics);
    printDebugLog("Created Graphics Command Pool");

    // Compute command pool
    if (queueFamilyIndices.compute != queueFamilyIndices.graphics) {
        m_computeCommandPool = createCommandPool(queueFamilyIndices.compute);
        printDebugLog("Created Compute Command Pool");
    } else {
        m_computeCommandPool = m_graphicsCommandPool;
    }

    // Transfer command pool
    if (queueFamilyIndices.transfer != queueFamilyIndices.graphics && queueFamilyIndices.transfer != queueFamilyIndices.compute) {
        m_transferCommandPool = createCommandPool(queueFamilyIndices.transfer);
        printDebugLog("Created Transfer Command Pool");
    } else if (queueFamilyIndices.transfer != queueFamilyIndices.compute) {
        m_transferCommandPool = m_computeCommandPool;
    } else {
        m_transferCommandPool = m_graphicsCommandPool;
    }
}

VkCommandPool VulkanRenderer::createCommandPool(const uint32_t iQueueFamilyIndex)
{
    QVulkanDeviceFunctions* pDeviceFunctions = m_graphicDevice->getVulkanDeviceFunctions();
    Q_ASSERT(pDeviceFunctions != nullptr);

    VkCommandPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.queueFamilyIndex = iQueueFamilyIndex;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkCommandPool commandPool;
    VkResult result = pDeviceFunctions->vkCreateCommandPool(m_graphicDevice->getDevice(), &createInfo, nullptr, &commandPool);
    if (result != VK_SUCCESS) throw std::runtime_error("Failed to create a Command Pool");

    return commandPool;
}

void VulkanRenderer::createCommandBuffers()
{
    printDebugLog("Create Command buffers");

    QVulkanDeviceFunctions* pDeviceFunctions = m_graphicDevice->getVulkanDeviceFunctions();
    Q_ASSERT(pDeviceFunctions != nullptr);

    // Resize command buffer count to have one for each frame buffer
    m_graphicsCommandBuffers.resize(m_swapChain->getSwapchainImageCount());

    // Command buffer allocation information
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = m_graphicsCommandPool;
    // VK_COMMAND_BUFFER_LEVEL_PRIMARY: Buffer you submit directly to queue. Can't be called by other buffers.
    // VK_COMMAND_BUFFER_LEVEL_SECONDARY: Buffer can't be called directly. Can be called from other buffers via "vkCmdExecuteCommands" when recording commands in primary buffer.
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(m_graphicsCommandBuffers.size());

    // Allocate command buffers and place handles in array of buffers
    VkResult result = pDeviceFunctions->vkAllocateCommandBuffers(m_graphicDevice->getDevice(), &commandBufferAllocateInfo, m_graphicsCommandBuffers.data());
    if (result != VK_SUCCESS) throw std::runtime_error("Failed to allocate Command Buffers");
}

void VulkanRenderer::createUniformBuffers()
{
    printDebugLog("Create Uniform buffers");

    // MVP buffer size
    const VkDeviceSize mvpSize = sizeof(UboModelViewProjection);

    m_uboMvpBuffers.reserve(m_swapChain->getSwapchainImageCount());

    for (size_t i = 0; i < m_swapChain->getSwapchainImageCount(); ++i) {
        m_uboMvpBuffers.push_back(std::move(std::make_unique<Buffer>(m_graphicDevice.get())));
        m_uboMvpBuffers[i]->createBuffer(mvpSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }
}

void VulkanRenderer::destroyUniformBuffers()
{
    for (size_t i = 0; i < m_uboMvpBuffers.size(); ++i) {
        m_uboMvpBuffers[i]->destroy();
    }

    m_uboMvpBuffers.clear();
}

void VulkanRenderer::createDescriptorPool()
{
    printDebugLog("Create Descriptor pool");

    QVulkanDeviceFunctions* pDeviceFunctions = m_graphicDevice->getVulkanDeviceFunctions();
    Q_ASSERT(pDeviceFunctions != nullptr);

    // CREATE UNIFORM BUFFER DESCRIPTOR POOL
    // Type of descriptor + how many Descriptoprs, not Descriptor Sets (combined makes the pool size)
    VkDescriptorPoolSize mvpDescriptorPoolSize{};
    mvpDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;                         // Type of descriptor (uniform buffer, image sampler, etc)
    mvpDescriptorPoolSize.descriptorCount = static_cast<uint32_t>(m_uboMvpBuffers.size());  // Number of descriptor of this type

    // List of descriptor pool sizes
    std::vector<VkDescriptorPoolSize> descriptorPoolSizes = {mvpDescriptorPoolSize};

    // Descriptor pool creation information
    VkDescriptorPoolCreateInfo mvpDescriptorPoolCreateInfo{};
    mvpDescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;                       // Type of the structures
    mvpDescriptorPoolCreateInfo.maxSets = static_cast<uint32_t>(m_swapChain->getSwapchainImageCount());      // Maximum number of descriptor sets that can be allocated from pool
    mvpDescriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());           // Amount of pool sizes being passed
    mvpDescriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();                                     // Pool sizes to create pool with

    // Create Descriptor pool
    VkResult result = pDeviceFunctions->vkCreateDescriptorPool(m_graphicDevice->getDevice(), &mvpDescriptorPoolCreateInfo, nullptr, &m_descriptorPool);
    if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Descriptor pool");
}

void VulkanRenderer::destroyDescriptorPool()
{
    QVulkanDeviceFunctions* pDeviceFunctions = m_graphicDevice->getVulkanDeviceFunctions();
    Q_ASSERT(pDeviceFunctions != nullptr);

    if (m_descriptorPool != VK_NULL_HANDLE) {
        pDeviceFunctions->vkDestroyDescriptorPool(m_graphicDevice->getDevice(), m_descriptorPool, nullptr);
        m_descriptorPool = VK_NULL_HANDLE;
    }

    m_descriptorSets.clear();
}

void VulkanRenderer::createDescriptorSets()
{
    printDebugLog("Create Descriptor sets");

    QVulkanDeviceFunctions* pDeviceFunctions = m_graphicDevice->getVulkanDeviceFunctions();
    Q_ASSERT(pDeviceFunctions != nullptr);

    // Resize descriptor sets so one for every unifrom buffer
    m_descriptorSets.resize(m_swapChain->getSwapchainImageCount());

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts(m_swapChain->getSwapchainImageCount(), m_descriptorSetLayout);

    // Descriptor set allocation information
    VkDescriptorSetAllocateInfo descriptorSetAllocateinfo{};
    descriptorSetAllocateinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateinfo.descriptorPool = m_descriptorPool;                                                  // Pool to allocate Descriptor sets from
    descriptorSetAllocateinfo.descriptorSetCount = static_cast<uint32_t>(m_swapChain->getSwapchainImageCount());  // Number of descriptor sets to allocate
    descriptorSetAllocateinfo.pSetLayouts = descriptorSetLayouts.data();                                          // Layouts to use for each allocated set

    VkResult result = pDeviceFunctions->vkAllocateDescriptorSets(m_graphicDevice->getDevice(), &descriptorSetAllocateinfo, m_descriptorSets.data());
    if (result != VK_SUCCESS) throw std::runtime_error("Failed to create Descriptor sets");

    // Update descritor sets with buffer bindings
    for (size_t i = 0; i < m_swapChain->getSwapchainImageCount(); ++i) {
        // UBO modelViewProejction descriptor
        VkDescriptorBufferInfo mvpUniformBufferInfo{};

        mvpUniformBufferInfo.buffer = m_uboMvpBuffers[i]->getBuffer();     // Buffer to bind to descriptor set
        mvpUniformBufferInfo.offset = 0;                                   // Offset in buffer to start of data
        mvpUniformBufferInfo.range = sizeof(UboModelViewProjection);       // Size of data to bind

        // Data about connection between buffer and binding in shader
        VkWriteDescriptorSet mvpUniformBufferWriteDescriptorSet{};
        mvpUniformBufferWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mvpUniformBufferWriteDescriptorSet.dstSet = m_descriptorSets[i];                        // Descriptor set to write to
        mvpUniformBufferWriteDescriptorSet.dstBinding = 0;                                      // Binding in shader where data will be read
        mvpUniformBufferWriteDescriptorSet.dstArrayElement = 0;                                 // Index in array to write to
        mvpUniformBufferWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;  // Type of descriptor
        mvpUniformBufferWriteDescriptorSet.descriptorCount = 1;                                 // Number of descriptors t write
        mvpUniformBufferWriteDescriptorSet.pBufferInfo = &mvpUniformBufferInfo;                 // Buffer info

        // List of write descriptor sets
        std::vector<VkWriteDescriptorSet> writeDescriptorSets = {mvpUniformBufferWriteDescriptorSet};

        // Update the descriptor set with new buffer/binding info
        pDeviceFunctions->vkUpdateDescriptorSets(m_graphicDevice->getDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
    }
}

void VulkanRenderer::createSynchronization()
{
    printDebugLog("Create Synchronization");

    QVulkanDeviceFunctions* pDeviceFunctions = m_graphicDevice->getVulkanDeviceFunctions();
    Q_ASSERT(pDeviceFunctions != nullptr);

    m_imagesAvailable.resize(MAX_FRAMES_IN_FLIGHT,     VK_NULL_HANDLE);
    m_fences.resize(MAX_FRAMES_IN_FLIGHT,              VK_NULL_HANDLE);

    // Semaphore creation information
    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    // Fence creation information
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < m_fences.size(); ++i) {
        if ((pDeviceFunctions->vkCreateFence(m_graphicDevice->getDevice(), &fenceCreateInfo, nullptr, &m_fences[i])) != VK_SUCCESS) {
            throw std::runtime_error("Failed to fences");
        }
    }

    for (size_t i = 0; i < m_imagesAvailable.size(); ++i) {
        if ((pDeviceFunctions->vkCreateSemaphore(m_graphicDevice->getDevice(), &semaphoreCreateInfo, nullptr, &m_imagesAvailable[i])) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create semamphores");
        }
    }

    createRenderFinishedSemaphores();
}

void VulkanRenderer::createRenderFinishedSemaphores()
{
    printDebugLog("Create RenderedFinished sempaphores");

    QVulkanDeviceFunctions* pDeviceFunctions = m_graphicDevice->getVulkanDeviceFunctions();
    Q_ASSERT(pDeviceFunctions != nullptr);

    // Resize
    m_renderFinished.resize(m_swapChain->getSwapchainImageCount(), VK_NULL_HANDLE);


    // Semaphore re-creation information
    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (size_t i = 0; i < m_renderFinished.size(); ++i) {
        if ((pDeviceFunctions->vkCreateSemaphore(m_graphicDevice->getDevice(), &semaphoreCreateInfo, nullptr, &m_renderFinished[i])) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create semamphores");
        }
    }
}
