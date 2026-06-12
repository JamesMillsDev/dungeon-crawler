#pragma once

#include <array>
#include <atomic>
#include <mutex>
#include <utility>
#include <vector>

#include "RenderCommand.h"

using std::array;
using std::atomic;
using std::mutex;
using std::scoped_lock;
using std::vector;

class RenderCommandBuffer
{
private:
	array<vector<RenderCommand>, 2> m_buffers;
	atomic<uint32_t> m_writeIndex{ 0 };
	uint32_t m_readIndex{ 1 };
	mutex m_swapMutex;

public:
	// Called from any game thread at any time - only contends with other pushers
	void Push(RenderCommand command);

	// Called once per frame by the render thread before recording begins.
	// After this, game threads write to the old read buffer and
	// the render thread owns the old write buffer - no further contention.
	void Swap();

	// No lock needed - render thread has exclusive ownership after Swap()
	[[nodiscard]] const vector<RenderCommand>& Read() const;
};