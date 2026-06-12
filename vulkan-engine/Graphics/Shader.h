#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan.h>

using std::string;
using std::vector;

class Shader
{
private:
	static vector<char> ReadShaderFile(const string& fileName);

private:
	string m_fileName;
	VkDevice m_device;
	VkShaderModule m_module;

public:
	explicit Shader(string fileName);
	~Shader();

public:
	void Load(VkDevice device);

	[[nodiscard]] VkShaderModule GetModule() const;

private:
	[[nodiscard]] VkShaderModule CreateShaderModule() const;

};