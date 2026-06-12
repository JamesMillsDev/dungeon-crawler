#pragma once

#include "VulkanCommon.h"

class VulkanDevice;

class VulkanRenderPass
{
	friend class Renderer;

private:
	VkRenderPass m_renderPass;
	VkFormat m_swapChainImageFormat;

	VulkanDevice* m_device;

private:
	VulkanRenderPass(VulkanDevice* device, VkFormat swapChainFormat);

private:
	void Create();
	void Cleanup();

};