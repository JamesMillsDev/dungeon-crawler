#pragma once

#include <vulkan/vulkan.h>

class Config;
class VulkanInstance;
class Window;

class Renderer
{
	friend class Application;

private:
	VulkanInstance* m_vulkanInstance;
	Window* m_window;

private:
	Renderer(Config* config, Window* window);
	~Renderer();

private:
	void Initialise();
	void Cleanup();

};