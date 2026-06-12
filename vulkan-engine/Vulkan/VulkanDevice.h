#pragma once

#include "VulkanCommon.h"
#include "VulkanStructs.h"

class VulkanDevice
{
	friend class Renderer;

private:
	static bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

private:
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_logicalDevice;

	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;

	VkSurfaceKHR m_surface;

private:
	explicit VulkanDevice(VkSurfaceKHR surface);

public:
	[[nodiscard]] QueueFamilyIndices FindQueueFamilies() const;
	[[nodiscard]] SwapChainSupportDetails QuerySwapChainSupport() const;

	[[nodiscard]] VkPhysicalDevice Physical() const;
	[[nodiscard]] VkDevice Logical() const;

private:
	void Create(const VkInstance& instance);
	void Cleanup();

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;
	bool IsDeviceSuitable(VkPhysicalDevice device) const;
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;

};
