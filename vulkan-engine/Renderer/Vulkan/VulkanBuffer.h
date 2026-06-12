#pragma once

#include "VulkanCommon.h"

class VulkanDevice;

class VulkanBuffer
{
	friend class Renderer;

private:
	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;
	VkBufferUsageFlags m_usage;
	VkSharingMode m_sharing;
	VkMemoryPropertyFlags m_memoryFlags;

	size_t m_vertexSize;
	size_t m_vertexCount;

	VulkanDevice* m_device;

private:
	VulkanBuffer(VulkanDevice* device, size_t vertexSize, size_t vertexCount, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryFlags, VkSharingMode sharing = VK_SHARING_MODE_EXCLUSIVE);

public:
	void Fill(const void* bufferData) const;

	[[nodiscard]] VkBuffer Get() const;

private:
	void Create();
	void Cleanup();

	[[nodiscard]] uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

};