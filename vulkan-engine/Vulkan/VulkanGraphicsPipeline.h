#pragma once

#include <initializer_list>
#include <string>

#include "VulkanCommon.h"

using std::initializer_list;
using std::string;

class VulkanDevice;

struct ShaderInfo
{
	VkShaderStageFlagBits stage;
	string shader;
	string entryPoint = "main";
};

class VulkanGraphicsPipeline
{
	friend class Renderer;

private:
	VkPipelineLayout m_pipelineLayout;
	vector<VkPipeline> m_pipelines;

	VulkanDevice* m_device;
	VkRenderPass m_renderPass;

private:
	VulkanGraphicsPipeline(VulkanDevice* device, VkRenderPass renderPass);

private:
	void Create(const vector<initializer_list<ShaderInfo>>& shaderInfos);
	void Cleanup();

};
