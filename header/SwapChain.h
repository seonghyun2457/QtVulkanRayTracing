#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

class SwapChain
{
public:
    SwapChain();
    virtual ~SwapChain();

    SwapChain(const SwapChain& iOther) = delete;
    SwapChain& operator=(const SwapChain& iOther) = delete;

    SwapChain(SwapChain&& iOther) = delete;
    SwapChain& operator=(SwapChain&& iOther) = delete;


};

#endif // SWAPCHAIN_H
