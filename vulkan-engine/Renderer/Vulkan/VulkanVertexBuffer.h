#pragma once

#include "VulkanCommon.h"

class VulkanDevice;

class VulkanVertexBuffer
{
	friend class Renderer;

private:
	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;

	size_t m_vertexSize;
	size_t m_vertexCount;

	VulkanDevice* m_device;

private:
	VulkanVertexBuffer(size_t vertexSize, size_t vertexCount, VulkanDevice* device);

public:
	void Fill(const void* vertices) const;

	VkBuffer Get() const;

private:
	void Create();
	void Cleanup();

	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

};