#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <vector>

#include <vulkan/vulkan.h>

using std::atomic;
using std::condition_variable;
using std::mutex;
using std::queue;
using std::vector;

struct FrameSubmission
{
	vector<VkCommandBuffer> commandBuffers;
	uint32_t imageIndex;
	uint32_t frameIndex;
};

class RenderSubmissionQueue
{
	friend class Renderer;

private:
	queue<FrameSubmission> m_queue;
	mutex m_mutex;
	condition_variable m_cv;
	atomic<bool> m_running;

private:
	RenderSubmissionQueue();

public:
	void Push(FrameSubmission submission);

	// Blocks until a submission is available or Shutdown() is called.
	// Returns false when shutting down with nothing left to process.
	bool Pop(FrameSubmission& out);

	void Shutdown();

};