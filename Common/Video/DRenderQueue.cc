
#include "DRenderQueue.h"

DRenderQueue::DRenderQueue() 
{
    m_stoped.store(false);
    m_frame_in_queue.store(false);
}

DRenderQueue::~DRenderQueue() {

}

DVoid DRenderQueue::Start()
{
    m_thread = std::thread(std::bind(&DRenderQueue::MessageLoopThread, this));
}

DVoid DRenderQueue::Stop()
{
    m_stoped.store(true);
    Notify();
    m_thread.join();
}

DVoid DRenderQueue::Setup(DVoid* wnd, DRect& rect)
{
    m_context = wnd;
    m_destRect = rect;
}

DInt32 DRenderQueue::GetQueueSize() 
{
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    return m_queue.size();
}

DVoid DRenderQueue::MessageLoopThread() 
{
    while (true) 
    {
        {
            std::unique_lock<std::mutex> lock(m_cv_mutex);
            m_cv.wait(lock, [&] { return m_frame_in_queue.load(); });
        }

        ProcessFrame();

        if (m_stoped.load()) {
            break;
        }
    }
}

DVoid DRenderQueue::ProcessFrame()
{
    while (GetQueueSize() > 0) 
    {
        DVideoFrame cur_frame;
        {
            std::lock_guard<std::mutex> lock(m_queue_mutex);
            cur_frame = m_queue.front();
            m_queue.pop_front();
        }
        
    }
    m_frame_in_queue.store(false);
}

DVoid DRenderQueue::Notify()
{
    m_frame_in_queue.store(true);
    m_cv.notify_one();
}

DInt32 DRenderQueue::PushFrame(DVideoFrame frame)
{
    {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        m_queue.push_back(frame);
    }

    Notify();
    return 0;
}

