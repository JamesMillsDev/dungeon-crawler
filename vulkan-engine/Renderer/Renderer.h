#pragma once

#include <thread>

#include "RenderCommandBuffer.h"
#include "RenderSubmissionQueue.h"

class VulkanCommandManager;
class VulkanDevice;
class VulkanGraphicsPipeline;
class VulkanInstance;
class VulkanRenderPass;
class VulkanSwapChain;
class VulkanSyncObjects;

class Config;
class Window;

using std::thread;

class Renderer
{
	friend class Application;

private:
	Window* m_window;

	VulkanInstance* m_vulkanInstance;
	VulkanDevice* m_vulkanDevice;
	VulkanSwapChain* m_swapChain;
	VulkanRenderPass* m_renderPass;
	VulkanGraphicsPipeline* m_graphicsPipeline;
	VulkanCommandManager* m_commandManager;
	VulkanSyncObjects* m_syncObjects;

	// Double-buffered command list - game threads push here, render thread reads after Swap()
	RenderCommandBuffer m_renderCommandBuffer;

	// Submission thread machinery
	RenderSubmissionQueue m_submissionQueue;
	thread m_submissionThread;

private:
	Renderer(Config* config, Window* window);
	~Renderer();

private:
	void Initialise();
	void Cleanup();

	// Called each frame - safe to call from any thread
	void RenderFrame();

	// Called from game/entity threads to enqueue draw calls
	void DrawTexture(uint32_t textureId, float x, float y, float width, float height);
	void DrawMesh(uint32_t meshId, float x, float y, float z);

	// Runs on m_submissionThread - drains m_submissionQueue and calls SubmitQueue
	void SubmissionLoop();
};