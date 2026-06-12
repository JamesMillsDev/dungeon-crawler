#include "Renderer.h"

#include "Window.h"
#include "Graphics/Mesh.h"
#include "Vulkan/VulkanCommandManager.h"
#include "Vulkan/VulkanDevice.h"
#include "Vulkan/VulkanGraphicsPipeline.h"
#include "Vulkan/VulkanInstance.h"
#include "Vulkan/VulkanRenderPass.h"
#include "Vulkan/VulkanSwapChain.h"
#include "Vulkan/VulkanSyncObjects.h"
#include "Vulkan/VulkanBuffer.h"

VulkanBuffer* vertexBuffer = nullptr;

Renderer::Renderer(Config* config, Window* window)
	: m_window{ window },
	m_vulkanInstance{ new VulkanInstance{ config } },
	m_vulkanDevice{ nullptr },
	m_swapChain{ nullptr },
	m_renderPass{ nullptr },
	m_graphicsPipeline{ nullptr },
	m_commandManager{ nullptr },
	m_syncObjects{ nullptr }
{}

Renderer::~Renderer()
{
	delete vertexBuffer;
	vertexBuffer = nullptr;

	delete m_syncObjects;
	m_syncObjects = nullptr;

	delete m_commandManager;
	m_commandManager = nullptr;

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
				{.stage = VK_SHADER_STAGE_VERTEX_BIT,   .shader = "Triangle.vert" },
				{.stage = VK_SHADER_STAGE_FRAGMENT_BIT, .shader = "Triangle.frag" }
			}
		}
	);

	m_swapChain->CreateFrameBuffers(m_renderPass->m_renderPass);

	// One command pool/buffer set per pipeline - must be called after pipeline creation
	m_commandManager = new VulkanCommandManager{ m_vulkanDevice };
	m_commandManager->Create(static_cast<uint32_t>(m_graphicsPipeline->m_pipelines.size()));

	m_syncObjects = new VulkanSyncObjects{ m_vulkanDevice };
	m_syncObjects->Create();

	// Start the submission thread - runs until Cleanup() calls Shutdown()
	m_submissionThread = std::thread{ &Renderer::SubmissionLoop, this };

	vertexBuffer = new VulkanBuffer
	{ 
		m_vulkanDevice, sizeof(Vertex), vertices.size(), 
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	};
	vertexBuffer->Create();

	vertexBuffer->Fill(vertices.data());
}

void Renderer::Cleanup()
{
	// Signal the submission thread and wait for it to drain the queue
	m_submissionQueue.Shutdown();
	if (m_submissionThread.joinable())
	{
		m_submissionThread.join();
	}

	// Wait for the GPU to be fully idle before destroying resources
	vkDeviceWaitIdle(m_vulkanDevice->Logical());

	m_swapChain->Cleanup();

	vertexBuffer->Cleanup();

	m_syncObjects->Cleanup();
	m_commandManager->Cleanup();
	m_graphicsPipeline->Cleanup();
	m_renderPass->Cleanup();

	m_vulkanDevice->Cleanup();

	m_window->CleanupVulkan(m_vulkanInstance->m_instance);
	m_vulkanInstance->Cleanup();
}

void Renderer::RenderFrame()
{
	// Swap buffers first - game threads continue writing to the newly freed buffer
	// while the render thread records from the just-swapped read buffer
	m_renderCommandBuffer.Swap();

	const auto [imageIndex, frameIndex] = m_vulkanDevice->AcquireNextImage(m_syncObjects, m_swapChain);

	const vector<VkCommandBuffer> commandBuffers = m_commandManager->RecordAll(
		frameIndex,
		imageIndex,
		m_swapChain->m_swapChainFramebuffers,
		m_swapChain->m_swapChainExtent,
		m_renderPass->m_renderPass,
		m_graphicsPipeline->m_pipelines,
		m_renderCommandBuffer.Read()
	);

	m_submissionQueue.Push({
		.commandBuffers = commandBuffers,
		.imageIndex = imageIndex,
		.frameIndex = frameIndex
		});
}

void Renderer::DrawTexture(const uint32_t textureId, const float x, const float y, const float width, const float height)
{
	m_renderCommandBuffer.Push(DrawTextureCommand{
		.textureId = textureId,
		.x = x,
		.y = y,
		.width = width,
		.height = height
		});
}

void Renderer::DrawMesh(const uint32_t meshId, const float x, const float y, const float z)
{
	m_renderCommandBuffer.Push(DrawMeshCommand{
		.meshId = meshId,
		.x = x,
		.y = y,
		.z = z
		});
}

void Renderer::SubmissionLoop()
{
	FrameSubmission submission;
	while (m_submissionQueue.Pop(submission))
	{
		m_vulkanDevice->SubmitQueue(
			m_syncObjects,
			submission.commandBuffers,
			submission.imageIndex,
			submission.frameIndex,
			m_swapChain
		);
	}
}