#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <QVulkanInstance>

#include <vector>

class VulkanWindow;
class GraphicDevice;

typedef struct SwapchainDetails {
    VkSurfaceCapabilitiesKHR surfaceCapabilities;     // Surface properties, e.g. image size/extent
    std::vector<VkSurfaceFormatKHR> formats;          // Surface image formats, e.g. RGBA and size of each color
    std::vector<VkPresentModeKHR> presentationModes;   // How images should be presented to screen
} swapchainDetails_t;


typedef struct SwapchainImage {
    VkImage image{VK_NULL_HANDLE};
    VkImageView imageView{VK_NULL_HANDLE};
} swapchainImage_t;

class SwapChain
{
public:
    explicit SwapChain(VulkanWindow* ipWindow, GraphicDevice* ipGraphicDevice);
    virtual ~SwapChain();

    SwapChain(const SwapChain& iOther) = delete;
    SwapChain& operator=(const SwapChain& iOther) = delete;

    SwapChain(SwapChain&& iOther) = delete;
    SwapChain& operator=(SwapChain&& iOther) = delete;

    void createSwapchain(const VkSurfaceKHR& iSurface, VkSurfaceFormatKHR& oSurfaceFormat);
    void recreateSwapchain(const VkSurfaceKHR& iSurface, VkSurfaceFormatKHR& oSurfaceFormat);
    void destroy();

    std::vector<swapchainImage_t>& getSwapchainImages() { return m_swapchainImages; }
    inline const VkSwapchainKHR& getSwapchain() const { return m_swapchain; }
    inline const size_t getSwapchainImageCount() const { return m_swapchainImages.size(); }
    inline const VkExtent2D getExtent() const { return m_extent; }

private:
    void destroySwapchainImageViews();

    const swapchainDetails_t getSwapChainDetails(const VkSurfaceKHR& iSurface);
    const VkSurfaceFormatKHR chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& iFormats);
    const VkPresentModeKHR chooseBestPresentationMode(const std::vector<VkPresentModeKHR>& iPresentationModes);
    const VkExtent2D chooseSwapExtent(const uint32_t iWidth, const uint32_t iHeight, const VkSurfaceCapabilitiesKHR& iSurfaceCapabilities);
    const VkImageView createImageView(const VkImage iImage, const VkFormat iFormat, const VkImageAspectFlags iAspectFlags);

    // Print information
    void printDebugInfo(const QString& iString) const;

private:
    // Vulkan Widget
    VulkanWindow* m_pWindow{nullptr};

    // Physical Device
    GraphicDevice* m_pGraphicDevice{nullptr};


    VkSwapchainKHR m_swapchain{VK_NULL_HANDLE};
    VkSwapchainKHR m_oldSwapchain{VK_NULL_HANDLE};
    swapchainDetails_t m_swapchainDetails;
    std::vector<swapchainImage_t> m_swapchainImages;
    VkExtent2D m_extent{};
};

#endif // SWAPCHAIN_H
