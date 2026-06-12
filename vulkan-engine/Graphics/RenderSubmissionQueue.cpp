#include "RenderSubmissionQueue.h"

using std::scoped_lock;
using std::unique_lock;

RenderSubmissionQueue::RenderSubmissionQueue()
	: m_running{ true }
{

}

void RenderSubmissionQueue::Push(FrameSubmission submission)
{
	{
		scoped_lock lock{ m_mutex };
		m_queue.push(std::move(submission));
	}
	m_cv.notify_one();
}

bool RenderSubmissionQueue::Pop(FrameSubmission& out)
{
	unique_lock lock{ m_mutex };
	m_cv.wait(lock, [&]
		{
			return !m_queue.empty() || !m_running;
		});

	if (m_queue.empty())
	{
		return false;
	}

	out = std::move(m_queue.front());
	m_queue.pop();
	return true;
}

void RenderSubmissionQueue::Shutdown()
{
	m_running = false;
	m_cv.notify_all();
}