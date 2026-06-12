#pragma once

#include <mutex>

#include "VulkanCommon.h"

class VulkanDevice;

struct GLFWwindow;

using std::mutex;

class VulkanSwapChain
{
	friend class Renderer;
	friend VulkanDevice;

private:
	static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR ChooseSwapPresentMode(const vector<VkPresentModeKHR>& availablePresentModes);

private:
	VkSwapchainKHR m_swapChain;
	vector<VkImage> m_swapChainImages;
	VkFormat m_swapChainImageFormat;
	VkExtent2D m_swapChainExtent;
	vector<VkImageView> m_swapChainImageViews;
	vector<VkFramebuffer> m_swapChainFramebuffers;

	VulkanDevice* m_device;
	VkSurfaceKHR m_surface;
	GLFWwindow* m_windowHandle;

	mutable mutex m_accessMutex;

private:
	explicit VulkanSwapChain(VulkanDevice* logicalDevice, VkSurfaceKHR surface, GLFWwindow* windowHandle);

private:
	void CreateSwapChain();
	void CreateImageViews();
	void CreateFrameBuffers(VkRenderPass renderPass);

	void Cleanup() const;
	void Recreate(VkRenderPass renderPass);

	[[nodiscard]] VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

};