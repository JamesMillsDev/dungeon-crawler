#include "VulkanDevice.h"

#include <set>
#include <stdexcept>
#include <string>

#include "VulkanCommandManager.h"
#include "VulkanSwapchain.h"
#include "VulkanSyncObjects.h"

using std::runtime_error;
using std::scoped_lock;
using std::set;
using std::string;

bool VulkanDevice::CheckDeviceExtensionSupport(const VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	set<string> requiredExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());
	for (const auto& [extensionName, specVersion] : availableExtensions)
	{
		requiredExtensions.erase(extensionName);
	}

	return requiredExtensions.empty();
}

VulkanDevice::VulkanDevice(const VkSurfaceKHR surface)
	: m_physicalDevice{ VK_NULL_HANDLE }, m_logicalDevice{ VK_NULL_HANDLE },
	m_graphicsQueue{ VK_NULL_HANDLE }, m_presentQueue{ VK_NULL_HANDLE },
	m_surface{ surface }
{}

QueueFamilyIndices VulkanDevice::FindQueueFamilies() const
{
	return FindQueueFamilies(m_physicalDevice);
}

SwapChainSupportDetails VulkanDevice::QuerySwapChainSupport() const
{
	return QuerySwapChainSupport(m_physicalDevice);
}

VkPhysicalDevice VulkanDevice::Physical() const
{
	return m_physicalDevice;
}

VkDevice VulkanDevice::Logical() const
{
	return m_logicalDevice;
}

void VulkanDevice::Create(const VkInstance& instance)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw runtime_error("Failed to find GPUs with Vulkan support!");
	}

	vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for (const VkPhysicalDevice& device : devices)
	{
		if (IsDeviceSuitable(device))
		{
			m_physicalDevice = device;
			break;
		}
	}

	if (m_physicalDevice == VK_NULL_HANDLE)
	{
		throw runtime_error("Failed to find a suitable GPU!");
	}

	QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);

	vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	set uniqueQueueFamilies =
	{
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.emplace_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
	createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

	if (ENABLE_VALIDATION_LAYERS)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
		createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_logicalDevice) != VK_SUCCESS)
	{
		throw runtime_error("Failed to create logical device!");
	}

	vkGetDeviceQueue(m_logicalDevice, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_logicalDevice, indices.presentFamily.value(), 0, &m_presentQueue);
}

void VulkanDevice::Cleanup()
{
	vkDestroyDevice(m_logicalDevice, nullptr);
	m_logicalDevice = nullptr;
}

AcquireResult VulkanDevice::AcquireNextImage(VulkanSyncObjects* syncObjects, const VulkanSwapChain* swapChain) const
{
	const uint32_t frameIndex = syncObjects->m_currentFrame.load();

	vkWaitForFences(m_logicalDevice, 1, &syncObjects->m_inFlightFences[frameIndex], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	{
		scoped_lock lock{ swapChain->m_accessMutex };
		const VkResult result = vkAcquireNextImageKHR(
			m_logicalDevice,
			swapChain->m_swapChain,
			UINT64_MAX,
			syncObjects->m_imageAvailableSemaphores[frameIndex],
			VK_NULL_HANDLE,
			&imageIndex
		);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			throw runtime_error("Swap chain out of date - recreate required!");
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw runtime_error("Failed to acquire swap chain image!");
		}
	}

	syncObjects->m_currentFrame.store((frameIndex + 1) % MAX_FRAMES_IN_FLIGHT);
	return { .imageIndex = imageIndex, .frameIndex = frameIndex };
}

void VulkanDevice::SubmitQueue(
	const VulkanSyncObjects* syncObjects,
	const vector<VkCommandBuffer>& commandBuffers,
	const uint32_t imageIndex,
	const uint32_t frameIndex,
	const VulkanSwapChain* swapChain
) const
{
	const VkFence& currentFence = syncObjects->m_inFlightFences[frameIndex];
	const VkSemaphore& currentImageSemaphore = syncObjects->m_imageAvailableSemaphores[frameIndex];
	const VkSemaphore& currentFinishedSemaphore = syncObjects->m_renderFinishedSemaphores[frameIndex];

	vkResetFences(m_logicalDevice, 1, &currentFence);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &currentImageSemaphore;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
	submitInfo.pCommandBuffers = commandBuffers.data();
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &currentFinishedSemaphore;

	if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, currentFence) != VK_SUCCESS)
	{
		throw runtime_error("Failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &currentFinishedSemaphore;
	VkSwapchainKHR swapChains[] = { swapChain->m_swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	{
		scoped_lock lock{ swapChain->m_accessMutex };
		vkQueuePresentKHR(m_presentQueue, &presentInfo);
	}
}

QueueFamilyIndices VulkanDevice::FindQueueFamilies(const VkPhysicalDevice device) const
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const VkQueueFamilyProperties& queueFamily : queueFamilies)
	{
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);

		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		if (presentSupport)
		{
			indices.presentFamily = i;
		}

		if (indices.IsComplete())
		{
			break;
		}

		i++;
	}

	return indices;
}

bool VulkanDevice::IsDeviceSuitable(const VkPhysicalDevice device) const
{
	const QueueFamilyIndices indices = FindQueueFamilies(device);
	const bool extensionsSupported = CheckDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		const SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return indices.IsComplete() && extensionsSupported && swapChainAdequate;
}

SwapChainSupportDetails VulkanDevice::QuerySwapChainSupport(const VkPhysicalDevice device) const
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}