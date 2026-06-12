#include "Renderer.h"

#include "../Window.h"
#include "../Vulkan/VulkanDevice.h"
#include "../Vulkan/VulkanInstance.h"
#include "../Vulkan/VulkanGraphicsPipeline.h"
#include "../Vulkan/VulkanRenderPass.h"
#include "../Vulkan/VulkanSwapChain.h"

Renderer::Renderer(Config* config, Window* window) :
	m_window{ window }, m_vulkanInstance{ new VulkanInstance{ config } }, m_vulkanDevice{ nullptr },
	m_swapChain{ nullptr }, m_renderPass{ nullptr }, m_graphicsPipeline{ nullptr }
{}

Renderer::~Renderer()
{
	delete m_graphicsPipeline;
	m_graphicsPipeline = nullptr;

	delete m_renderPass;
	m_renderPass = nullptr;

	delete m_swapChain;
	m_swapChain = nullptr;

	delete m_vulkanDevice;
	m_vulkanDevice = nullptr;

	delete m_vulkanInstance;
	m_vulkanInstance = nullptr;
}

void Renderer::Initialise()
{
	m_vulkanInstance->Create();
	m_window->InitialiseVulkan(m_vulkanInstance->m_instance);

	m_vulkanDevice = new VulkanDevice{ m_window->m_surface };
	m_vulkanDevice->Create(m_vulkanInstance->m_instance);

	m_swapChain = new VulkanSwapChain{ m_vulkanDevice, m_window->m_surface, m_window->m_window };
	m_swapChain->CreateSwapChain();
	m_swapChain->CreateImageViews();

	m_renderPass = new VulkanRenderPass{ m_vulkanDevice, m_swapChain->m_swapChainImageFormat };
	m_renderPass->Create();

	m_graphicsPipeline = new VulkanGraphicsPipeline{ m_vulkanDevice, m_renderPass->m_renderPass };
	m_graphicsPipeline->Create(
		{
			{
				{ .stage = VK_SHADER_STAGE_VERTEX_BIT, .shader = "Triangle.vert" },
				{ .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .shader = "Triangle.frag" }
			}
		}
	);

	m_swapChain->CreateFrameBuffers(m_renderPass->m_renderPass);
}

void Renderer::Cleanup()
{
	m_swapChain->Cleanup();

	m_graphicsPipeline->Cleanup();
	m_renderPass->Cleanup();

	m_vulkanDevice->Cleanup();

	m_window->CleanupVulkan(m_vulkanInstance->m_instance);
	m_vulkanInstance->Cleanup();
}