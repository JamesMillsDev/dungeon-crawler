#include "Renderer.h"

#include "../Vulkan/VulkanInstance.h"
#include "../Window.h"

Renderer::Renderer(Config* config, Window* window)
	: m_vulkanInstance{ new VulkanInstance{ config } }, m_window{ window }
{
	
}

Renderer::~Renderer()
{
	delete m_vulkanInstance;
	m_vulkanInstance = nullptr;
}

void Renderer::Initialise()
{
	m_vulkanInstance->Initialise();
	m_window->InitialiseVulkan(m_vulkanInstance->m_instance);
}

void Renderer::Cleanup()
{
	m_window->CleanupVulkan(m_vulkanInstance->m_instance);
	m_vulkanInstance->Cleanup();
}