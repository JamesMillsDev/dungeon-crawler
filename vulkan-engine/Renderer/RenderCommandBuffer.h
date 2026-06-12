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

/**
 * @brief A thread-safe, double-buffered queue of render commands.
 *
 * Game and entity threads push draw requests here at any time during the frame.
 * At the start of each frame the render thread calls Swap(), which atomically
 * exchanges the read and write buffers. After the swap the render thread has
 * exclusive ownership of the read buffer and can iterate it without locks,
 * while game threads continue writing into the now-cleared write buffer with
 * only push-time contention against each other.
 */
class RenderCommandBuffer
{
private:
	/** @brief The two command lists alternated between each frame. */
	array<vector<RenderCommand>, 2> m_buffers;

	/**
	 * @brief Index into m_buffers that game threads currently write to.
	 *
	 * Atomic because Swap() reads and modifies it while game threads may
	 * simultaneously be inside Push().
	 */
	atomic<uint32_t> m_writeIndex{ 0 };

	/**
	 * @brief Index into m_buffers that the render thread currently reads from.
	 *
	 * Only ever accessed by the render thread, so no atomic is required.
	 */
	uint32_t m_readIndex{ 1 };

	/**
	 * @brief Protects index exchange in Swap() and the write during Push().
	 *
	 * The critical section in each is intentionally short: Swap() only
	 * exchanges two integers and clears a vector, Push() only appends one
	 * element. Contention is therefore minimal.
	 */
	mutex m_swapMutex;

public:
	/**
	 * @brief Enqueues a render command from any thread.
	 *
	 * Acquires m_swapMutex briefly to ensure the command lands in the
	 * correct buffer if Swap() is called concurrently. Contention only
	 * occurs against other callers of Push() or a concurrent Swap().
	 *
	 * @param command The render command to enqueue.
	 */
	void Push(RenderCommand command);

	/**
	 * @brief Exchanges the read and write buffers.
	 *
	 * Must be called exactly once per frame by the render thread before
	 * recording begins. After this returns:
	 * - The render thread owns the previous write buffer via Read().
	 * - Game threads write freely into the newly cleared write buffer.
	 *
	 * @pre  Called only from the render thread.
	 * @post Read() returns the buffer that was being written to this frame.
	 */
	void Swap();

	/**
	 * @brief Returns the command list owned by the render thread this frame.
	 *
	 * Valid to call without a lock after Swap() and until the next Swap().
	 * The returned reference is stable for the entire recording phase.
	 *
	 * @pre  Swap() has been called this frame.
	 * @return A read-only view of the render commands to record.
	 */
	[[nodiscard]] const vector<RenderCommand>& Read() const;
};