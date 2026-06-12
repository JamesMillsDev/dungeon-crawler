#include "VulkanSwapchain.h"

#include <algorithm>
#include <stdexcept>
#include <GLFW/glfw3.h>

#include "VulkanDevice.h"

using std::runtime_error;

VkSurfaceFormatKHR VulkanSwapChain::ChooseSwapSurfaceFormat(const vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR VulkanSwapChain::ChooseSwapPresentMode(const vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VulkanSwapChain::VulkanSwapChain(VulkanDevice* logicalDevice, const VkSurfaceKHR surface, GLFWwindow* windowHandle) :
	m_swapChain{ VK_NULL_HANDLE }, m_swapChainImageFormat{}, m_swapChainExtent{}, m_device{ logicalDevice },
	m_surface{ surface }, m_windowHandle{ windowHandle }
{}

void VulkanSwapChain::CreateSwapChain()
{
	auto [scCapabilities, scFormats, scPresentModes] = m_device->QuerySwapChainSupport();

	const VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(scFormats);
	const VkPresentModeKHR presentMode = ChooseSwapPresentMode(scPresentModes);
	const VkExtent2D extent = ChooseSwapExtent(scCapabilities);

	uint32_t imageCount = scCapabilities.minImageCount + 1;

	if (scCapabilities.maxImageCount > 0 && imageCount > scCapabilities.maxImageCount)
	{
		imageCount = scCapabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	auto [graphicsFamily, presentFamily] = m_device->FindQueueFamilies();
	const uint32_t queueFamilyIndices[] = { graphicsFamily.value(), presentFamily.value() };

	if (graphicsFamily != presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = scCapabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(m_device->Logical(), &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
	{
		throw runtime_error("Failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(m_device->Logical(), m_swapChain, &imageCount, nullptr);
	m_swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device->Logical(), m_swapChain, &imageCount, m_swapChainImages.data());

	m_swapChainImageFormat = surfaceFormat.format;
	m_swapChainExtent = extent;
}

void VulkanSwapChain::CreateImageViews()
{
	m_swapChainImageViews.resize(m_swapChainImages.size());

	for (size_t i = 0; i < m_swapChainImages.size(); ++i)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_swapChainImages[i];

		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_swapChainImageFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(m_device->Logical(), &createInfo, nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS)
		{
			throw runtime_error("Failed to create image views!");
		}
	}
}

void VulkanSwapChain::CreateFrameBuffers(const VkRenderPass renderPass)
{
	m_swapChainFramebuffers.resize(m_swapChainImageViews.size());

	for (size_t i = 0; i < m_swapChainImageViews.size(); i++)
	{
		const VkImageView attachments[] = { m_swapChainImageViews[i] };

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = m_swapChainExtent.width;
		framebufferInfo.height = m_swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(m_device->Logical(), &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS)
		{
			throw runtime_error("Failed to create framebuffer!");
		}
	}
}

void VulkanSwapChain::Cleanup() const
{
	for (const VkFramebuffer& framebuffer : m_swapChainFramebuffers)
	{
		vkDestroyFramebuffer(m_device->Logical(), framebuffer, nullptr);
	}

	for (const VkImageView& imageView : m_swapChainImageViews)
	{
		vkDestroyImageView(m_device->Logical(), imageView, nullptr);
	}

	vkDestroySwapchainKHR(m_device->Logical(), m_swapChain, nullptr);
}

void VulkanSwapChain::Recreate(const VkRenderPass renderPass)
{
	vkDeviceWaitIdle(m_device->Logical());

	Cleanup();

	CreateSwapChain();
	CreateImageViews();
	CreateFrameBuffers(renderPass);
}

VkExtent2D VulkanSwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}

	int width, height;
	glfwGetFramebufferSize(m_windowHandle, &width, &height);

	VkExtent2D actualExtent = 
	{
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)
	};

	actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return actualExtent;
}
