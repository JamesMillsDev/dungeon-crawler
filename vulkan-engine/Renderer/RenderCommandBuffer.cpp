#include "RenderCommandBuffer.h"

void RenderCommandBuffer::Push(RenderCommand command) 
{
	scoped_lock lock{ m_swapMutex };
	m_buffers[m_writeIndex].push_back(std::move(command));
}

void RenderCommandBuffer::Swap() 
{
	scoped_lock lock{ m_swapMutex };
	m_readIndex = m_writeIndex.load();
	m_writeIndex = (m_writeIndex + 1) % 2;
	m_buffers[m_writeIndex].clear();
}

const vector<RenderCommand>& RenderCommandBuffer::Read() const
{
	return m_buffers[m_readIndex];
}