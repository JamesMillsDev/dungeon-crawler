#include "Shader.h"

#include <format>
#include <fstream>
#include <stdexcept>

using std::ifstream;
using std::runtime_error;
using std::streamsize;

vector<char> Shader::ReadShaderFile(const string& fileName)
{
	ifstream file(std::format("Content/Shaders/{}.spv", fileName), std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw runtime_error("Failed to open file!");
	}

	const streamsize fileSize = file.tellg();
	vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

Shader::Shader(string fileName)
	: m_fileName{ std::move(fileName) }, m_device{ VK_NULL_HANDLE }, m_module{ VK_NULL_HANDLE }
{

}

Shader::~Shader()
{
	vkDestroyShaderModule(m_device, m_module, nullptr);
}

void Shader::Load(const VkDevice device)
{
	m_device = device;
	m_module = CreateShaderModule();
}

VkShaderModule Shader::GetModule() const
{
	return m_module;
}

VkShaderModule Shader::CreateShaderModule() const
{
	const vector<char> code = ReadShaderFile(m_fileName);

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw runtime_error("Failed to create shader module!");
	}

	return shaderModule;
}
