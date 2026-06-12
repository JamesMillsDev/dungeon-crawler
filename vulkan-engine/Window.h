#pragma once

#include <string>

#include <vulkan/vulkan_core.h>

using std::string;

struct GLFWwindow;

class Config;

class Window
{
	friend class Application;

private:
	int m_width;
	int m_height;
	string m_title;
	bool m_resizable;
	bool m_isOpen;

	GLFWwindow* m_window;
	VkSurfaceKHR m_surface;

private:
	explicit Window(Config* config);
	~Window();

public:
	[[nodiscard]] int Width() const;

	void SetWidth(int w);

	[[nodiscard]] int Height() const;

	void SetHeight(int h);

	[[nodiscard]] bool ShouldClose() const;

	void InitialiseVulkan(const VkInstance& instance);
	void CleanupVulkan(const VkInstance& instance);

private:
	void Open();

	void Close();

};