#include "VulkanVertexBuffer.h"

#include <stdexcept>

#include "VulkanDevice.h"

using std::runtime_error;

VulkanVertexBuffer::VulkanVertexBuffer(const size_t vertexSize, const size_t vertexCount, VulkanDevice* device)
	: m_vertexBuffer{ VK_NULL_HANDLE }, m_vertexBufferMemory{ VK_NULL_HANDLE }, m_vertexSize{ vertexSize },
	m_vertexCount{ vertexCount }, m_device{ device }
{

}

void VulkanVertexBuffer::Fill(const void* vertices) const
{
	void* data;
	vkMapMemory(m_device->Logical(), m_vertexBufferMemory, 0, m_vertexSize * m_vertexCount, 0, &data);
	memcpy(data, vertices, m_vertexSize * m_vertexCount);
	vkUnmapMemory(m_device->Logical(), m_vertexBufferMemory);
}

VkBuffer VulkanVertexBuffer::Get() const
{
	return m_vertexBuffer;
}

void VulkanVertexBuffer::Create()
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = m_vertexSize * m_vertexCount;
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(m_device->Logical(), &bufferInfo, nullptr, &m_vertexBuffer) != VK_SUCCESS)
	{
		throw runtime_error("Failed to create vertex buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_device->Logical(), m_vertexBuffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(
		memRequirements.memoryTypeBits, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	if (vkAllocateMemory(m_device->Logical(), &allocInfo, nullptr, &m_vertexBufferMemory) != VK_SUCCESS)
	{
		throw runtime_error("failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(m_device->Logical(), m_vertexBuffer, m_vertexBufferMemory, 0);
}

void VulkanVertexBuffer::Cleanup()
{
	vkDestroyBuffer(m_device->Logical(), m_vertexBuffer, nullptr);
	vkFreeMemory(m_device->Logical(), m_vertexBufferMemory, nullptr);
}

uint32_t VulkanVertexBuffer::FindMemoryType(const uint32_t typeFilter, const VkMemoryPropertyFlags properties) const
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(m_device->Physical(), &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw runtime_error("failed to find suitable memory type!");
}
