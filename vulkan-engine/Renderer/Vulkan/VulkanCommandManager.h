#pragma once

#include <vector>

#include "VulkanCommon.h"
#include "Renderer/RenderCommand.h"

using std::vector;

class VulkanDevice;

// One pool and one set of per-frame buffers per pipeline.
// Command pools are not thread-safe so each pipeline gets its own.
struct PipelineCommands
{
	VkCommandPool pool{ VK_NULL_HANDLE };
	vector<VkCommandBuffer> buffers; // one per frame in flight
};

class VulkanCommandManager
{
	friend class Renderer;
	friend VulkanDevice;

private:
	static void RecordCommand(
		VkCommandBuffer commandBuffer,
		uint32_t imageIndex,
		const vector<VkFramebuffer>& swapChainFrameBuffers,
		const VkExtent2D& swapChainExtent,
		const VkRenderPass& renderPass,
		const VkPipeline& pipeline,
		const vector<RenderCommand>& commands
	);

private:
	vector<PipelineCommands> m_pipelineCommands;
	VulkanDevice* m_device;

private:
	explicit VulkanCommandManager(VulkanDevice* device);

private:
	// pipelineCount must match the number of pipelines in VulkanGraphicsPipeline
	void Create(uint32_t pipelineCount);
	void Cleanup();

	// Records all pipelines in parallel, returns ordered command buffers
	// ready for a single vkQueueSubmit call.
	[[nodiscard]] vector<VkCommandBuffer> RecordAll(
		uint32_t frameIndex,
		uint32_t imageIndex,
		const vector<VkFramebuffer>& swapChainFrameBuffers,
		const VkExtent2D& swapChainExtent,
		const VkRenderPass& renderPass,
		const vector<VkPipeline>& pipelines,
		const vector<RenderCommand>& commands
	);
};