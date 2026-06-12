#include "VulkanGraphicsPipeline.h"

#include <stdexcept>

#include "VulkanDevice.h"
#include "Graphics/Shader.h"

using std::runtime_error;

VulkanGraphicsPipeline::VulkanGraphicsPipeline(VulkanDevice* device, const VkRenderPass renderPass) :
	m_pipelineLayout{ VK_NULL_HANDLE }, m_device{ device }, m_renderPass{ renderPass }
{}

void VulkanGraphicsPipeline::Create(const vector<initializer_list<ShaderInfo>>& shaderInfos)
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pushConstantRangeCount = 0;

	if (vkCreatePipelineLayout(m_device->Logical(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
	{
		throw runtime_error("failed to create pipeline layout!");
	}

	m_pipelines.reserve(shaderInfos.size());
	int pipelineIndex = 0;
	for (initializer_list shaderInfoSet : shaderInfos)
	{
		vector<Shader*> shaders;
		shaders.reserve(shaderInfoSet.size());

		for (size_t i = 0; i < shaderInfoSet.size(); ++i)
		{
			ShaderInfo info = *(shaderInfoSet.begin() + i);

			Shader* shader = new Shader{ info.shader };
			shader->Load(m_device->Logical());

			shaders.emplace_back(shader);
		}

		vector<VkPipelineShaderStageCreateInfo> shaderStages;
		shaderStages.reserve(shaderInfoSet.size());

		for (size_t i = 0; i < shaderInfoSet.size(); ++i)
		{
			const auto& [stage, shaderName, entryPoint] = *(shaderInfoSet.begin() + i);

			VkPipelineShaderStageCreateInfo stageCreateInfo{};
			stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stageCreateInfo.stage = stage;
			stageCreateInfo.module = shaders[i]->GetModule();
			stageCreateInfo.pName = entryPoint.c_str();

			shaderStages.emplace_back(stageCreateInfo);
		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		vector dynamicStates =
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = m_pipelineLayout;
		pipelineInfo.renderPass = m_renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		m_pipelines.emplace_back(nullptr);
		if (vkCreateGraphicsPipelines(m_device->Logical(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipelines[pipelineIndex++]) != VK_SUCCESS)
		{
			throw runtime_error("failed to create graphics pipeline!");
		}

		for (Shader* shader : shaders)
		{
			delete shader;
		}
	}
}

void VulkanGraphicsPipeline::Cleanup()
{
	for (const VkPipeline pipeline : m_pipelines)
	{
		vkDestroyPipeline(m_device->Logical(), pipeline, nullptr);
	}
	m_pipelines.clear();

	vkDestroyPipelineLayout(m_device->Logical(), m_pipelineLayout, nullptr);
}