#include "VulkanBuffer.h"

#include <stdexcept>

#include "VulkanDevice.h"

using std::runtime_error;

VulkanBuffer::VulkanBuffer(VulkanDevice* device, const size_t vertexSize, const size_t vertexCount, const VkBufferUsageFlags usage,
	const VkMemoryPropertyFlags memoryFlags, const VkSharingMode sharing) :
	m_vertexBuffer{ VK_NULL_HANDLE }, m_vertexBufferMemory{ VK_NULL_HANDLE }, m_usage{ usage }, m_sharing{ sharing },
	m_memoryFlags{ memoryFlags }, m_vertexSize{ vertexSize }, m_vertexCount{ vertexCount }, m_device{ device }
{}

void VulkanBuffer::Fill(const void* bufferData) const
{
	void* data;
	vkMapMemory(m_device->Logical(), m_vertexBufferMemory, 0, m_vertexSize * m_vertexCount, 0, &data);
	memcpy(data, bufferData, m_vertexSize * m_vertexCount);
	vkUnmapMemory(m_device->Logical(), m_vertexBufferMemory);
}

VkBuffer VulkanBuffer::Get() const
{
	return m_vertexBuffer;
}

void VulkanBuffer::Create()
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = m_vertexSize * m_vertexCount;
	bufferInfo.usage = m_usage;
	bufferInfo.sharingMode = m_sharing;

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
		m_memoryFlags
	);

	if (vkAllocateMemory(m_device->Logical(), &allocInfo, nullptr, &m_vertexBufferMemory) != VK_SUCCESS)
	{
		throw runtime_error("failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(m_device->Logical(), m_vertexBuffer, m_vertexBufferMemory, 0);
}

void VulkanBuffer::Cleanup()
{
	vkDestroyBuffer(m_device->Logical(), m_vertexBuffer, nullptr);
	vkFreeMemory(m_device->Logical(), m_vertexBufferMemory, nullptr);
}

uint32_t VulkanBuffer::FindMemoryType(const uint32_t typeFilter, const VkMemoryPropertyFlags properties) const
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
