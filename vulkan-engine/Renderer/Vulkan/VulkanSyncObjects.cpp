#include "VulkanSyncObjects.h"

#include <stdexcept>

#include "VulkanDevice.h"

using std::runtime_error;

VulkanSyncObjects::VulkanSyncObjects(VulkanDevice* device)
	: m_currentFrame{ 0 }, m_device{ device }
{}

void VulkanSyncObjects::Create()
{
	m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(m_device->Logical(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(m_device->Logical(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(m_device->Logical(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
		{
			throw runtime_error("Failed to create synchronization objects for a frame!");
		}
	}
}

void VulkanSyncObjects::Cleanup()
{
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(m_device->Logical(), m_renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(m_device->Logical(), m_imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(m_device->Logical(), m_inFlightFences[i], nullptr);
	}
}