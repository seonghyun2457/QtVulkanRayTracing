#include "VulkanRenderer.h"

#include "VulkanWindow.h"
#include "Shader.h"

#include <QDebug>
#include <QVariant>

#include <exception>

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
        QVulkanDeviceFunctions* pFunctions = pVulkanInstance->deviceFunctions(m_graphicDevice->getDevice());

        // Wait until Idle status
        pFunctions->vkDeviceWaitIdle(m_graphicDevice->getDevice());


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

    QVulkanInstance* pVulkanInstance = m_pWindow->vulkanInstance();
    QVulkanDeviceFunctions* pDeviceFunctions = pVulkanInstance->deviceFunctions(m_graphicDevice->getDevice());

    // Wait until idle status
    pDeviceFunctions->vkDeviceWaitIdle(m_graphicDevice->getDevice());

    const size_t prevSwapChainImageCount = m_swapChain->getSwapchainImageCount();
    m_swapChain->recreateSwapchain(m_surface, m_surfaceFormat);

    if (prevSwapChainImageCount != m_swapChain->getSwapchainImageCount()) {
        // Destroy resources
        {

        }

        // Recreate resources
        {

        }
    }
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

    m_swapChain = std::make_unique<SwapChain>(m_pWindow, m_graphicDevice->getPhysicalDevice(), m_graphicDevice->getDevice());

    m_swapChain->createSwapchain(m_surface, m_surfaceFormat);
}

void VulkanRenderer::createDescriptorSetLayout()
{
    printDebugLog("Create Descriptor set layout");

    QVulkanInstance* pVulkanInstance = m_pWindow->vulkanInstance();
    QVulkanDeviceFunctions* pDeviceFunctions = pVulkanInstance->deviceFunctions(m_graphicDevice->getDevice());

    Q_ASSERT(pVulkanInstance && pVulkanInstance->isValid() && pDeviceFunctions);

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

    QVulkanInstance* pVulkanInstance = m_pWindow->vulkanInstance();
    QVulkanDeviceFunctions* pDeviceFunctions = pVulkanInstance->deviceFunctions(m_graphicDevice->getDevice());


    // Graphics pipeline creation info requres the array of shader stage creation info
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    // HANDLE SHADERS
    // Build shader modules to link to Graphics pipeline
    VkShaderModule vertexShaderModule = Shader::createShaderModule(pVulkanInstance, m_graphicDevice->getDevice(), "../../shaders/vert.spv");
    VkShaderModule fragmentShaderModule = Shader::createShaderModule(pVulkanInstance, m_graphicDevice->getDevice(), "../../shaders/frag.spv");

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

    Shader::destroyShaderModule(pVulkanInstance, m_graphicDevice->getDevice(), fragmentShaderModule);
    Shader::destroyShaderModule(pVulkanInstance, m_graphicDevice->getDevice(), vertexShaderModule);
}

