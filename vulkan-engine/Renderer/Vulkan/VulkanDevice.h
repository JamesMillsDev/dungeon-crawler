#pragma once

#include "VulkanCommon.h"
#include "VulkanStructs.h"

class VulkanCommandManager;
class VulkanSyncObjects;
class VulkanSwapChain;
class VulkanRenderPass;

// Returned by AcquireNextImage - both indices are needed downstream
struct AcquireResult
{
	uint32_t imageIndex;
	uint32_t frameIndex;
};

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

	// Called on the recording thread. Waits on the frame fence, acquires the
	// next swap chain image, then advances m_currentFrame. Returns both indices
	// so the caller can pass them through to SubmitQueue via FrameSubmission.
	[[nodiscard]] AcquireResult AcquireNextImage(VulkanSyncObjects* syncObjects, const VulkanSwapChain* swapChain) const;

	// Called on the submission thread. Resets the fence, submits recorded
	// command buffers, and presents. Does NOT touch m_currentFrame.
	void SubmitQueue(
		const VulkanSyncObjects* syncObjects,
		const vector<VkCommandBuffer>& commandBuffers,
		uint32_t imageIndex,
		uint32_t frameIndex,
		const VulkanSwapChain* swapChain
	) const;

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;
	bool IsDeviceSuitable(VkPhysicalDevice device) const;
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;
};