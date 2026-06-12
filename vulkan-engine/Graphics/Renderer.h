#pragma once

class VulkanDevice;
class VulkanGraphicsPipeline;
class VulkanInstance;
class VulkanSwapChain;
class VulkanRenderPass;

class Config;
class Window;

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

private:
	Renderer(Config* config, Window* window);
	~Renderer();

private:
	void Initialise();
	void Cleanup();

};