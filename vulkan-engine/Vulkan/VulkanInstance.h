#pragma once

#include <string>

#include "VulkanCommon.h"

using std::string;

class Config;
class Version;

class VulkanInstance
{
	friend class Renderer;

private:
	static bool CheckValidationLayerSupport();
	static vector<const char*> GetRequiredExtensions();
	static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

private:
	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;

	string m_engineTitle;
	Version* m_engineVersion;
	string m_appTitle;
	Version* m_appVersion;

private:
	explicit VulkanInstance(Config* config);
	~VulkanInstance();

private:
	void Initialise();
	void Cleanup();

};