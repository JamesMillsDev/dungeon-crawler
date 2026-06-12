#include "VulkanCommandManager.h"

#include <stdexcept>
#include <thread>

#include "VulkanDevice.h"
#include "VulkanStructs.h"

using std::runtime_error;
using std::thread;

void VulkanCommandManager::RecordCommand(
	const VkCommandBuffer commandBuffer,
	const uint32_t imageIndex,
	const vector<VkFramebuffer>& swapChainFrameBuffers,
	const VkExtent2D& swapChainExtent,
	const VkRenderPass& renderPass,
	const VkPipeline& pipeline,
	const vector<RenderCommand>& commands
)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw runtime_error("Failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = swapChainFrameBuffers[imageIndex];
	renderPassInfo.renderArea.offset = { .x = 0, .y = 0 };
	renderPassInfo.renderArea.extent = swapChainExtent;

	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapChainExtent.width);
	viewport.height = static_cast<float>(swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { .x = 0, .y = 0 };
	scissor.extent = swapChainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	// Translate each render command into Vulkan draw calls.
	// Vertex/index buffer binding and descriptor set updates go here
	// once resources are in place.
	for (const RenderCommand& command : commands)
	{
		std::visit([&]<typename T0>	(const T0& cmd)
			{
				using T = std::decay_t<T0>;

				if constexpr (std::is_same_v<T, DrawTextureCommand>)
				{
					// TODO: bind texture descriptor set, push position/size constants
					vkCmdDraw(commandBuffer, 6, 1, 0, 0); // quad = 6 verts
				}
				else if constexpr (std::is_same_v<T, DrawMeshCommand>)
				{
					// TODO: bind mesh vertex/index buffer, push transform constants
					vkCmdDraw(commandBuffer, 3, 1, 0, 0);
				}
			}, command);
	}

	// Fallback draw while render commands are still being wired up
	if (commands.empty())
	{
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	}

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw runtime_error("Failed to record command buffer!");
	}
}

VulkanCommandManager::VulkanCommandManager(VulkanDevice* device)
	: m_device{ device }
{}

void VulkanCommandManager::Create(const uint32_t pipelineCount)
{
	const auto [graphicsFamily, presentFamily] = m_device->FindQueueFamilies();

	m_pipelineCommands.resize(pipelineCount);

	for (PipelineCommands& pc : m_pipelineCommands)
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = graphicsFamily.value();

		if (vkCreateCommandPool(m_device->Logical(), &poolInfo, nullptr, &pc.pool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create command pool!");
		}

		pc.buffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = pc.pool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(pc.buffers.size());

		if (vkAllocateCommandBuffers(m_device->Logical(), &allocInfo, pc.buffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate command buffers!");
		}
	}
}

void VulkanCommandManager::Cleanup()
{
	for (auto& [pool, buffers] : m_pipelineCommands)
	{
		vkDestroyCommandPool(m_device->Logical(), pool, nullptr);
		pool = VK_NULL_HANDLE;
		buffers.clear();
	}

	m_pipelineCommands.clear();
}

vector<VkCommandBuffer> VulkanCommandManager::RecordAll(
	const uint32_t frameIndex,
	const uint32_t imageIndex,
	const vector<VkFramebuffer>& swapChainFrameBuffers,
	const VkExtent2D& swapChainExtent,
	const VkRenderPass& renderPass,
	const vector<VkPipeline>& pipelines,
	const vector<RenderCommand>& commands
)
{
	vector<thread> threads;
	threads.reserve(pipelines.size());

	for (size_t i = 0; i < pipelines.size(); ++i)
	{
		threads.emplace_back([&, i]
		{
				const VkCommandBuffer cmd = m_pipelineCommands[i].buffers[frameIndex];
				vkResetCommandBuffer(cmd, 0);

				RecordCommand(cmd,
					imageIndex,
					swapChainFrameBuffers,
					swapChainExtent,
					renderPass,
					pipelines[i],
					commands
				);
			});
	}

	for (std::thread& t : threads)
	{
		t.join();
	}

	// Collect in pipeline order - submission order matters for execution
	vector<VkCommandBuffer> result;
	result.reserve(pipelines.size());

	for (size_t i = 0; i < pipelines.size(); ++i)
	{
		result.emplace_back(m_pipelineCommands[i].buffers[frameIndex]);
	}

	return result;
}
