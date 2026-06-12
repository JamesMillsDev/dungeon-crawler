#pragma once

#include <atomic>

#include "VulkanCommon.h"

class VulkanDevice;

using std::atomic;

class VulkanSyncObjects
{
	friend class Renderer;
	friend VulkanDevice;

private:
	vector<VkSemaphore> m_imageAvailableSemaphores;
	vector<VkSemaphore> m_renderFinishedSemaphores;
	vector<VkFence> m_inFlightFences;

	// Atomic because AcquireNextImage (recording thread) reads and increments it,
	// while the submission thread holds a snapshot via frameIndex in FrameSubmission.
	atomic<uint32_t> m_currentFrame{ 0 };

	VulkanDevice* m_device;

private:
	explicit VulkanSyncObjects(VulkanDevice* device);

private:
	void Create();
	void Cleanup();
};