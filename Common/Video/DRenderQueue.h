#pragma once

#include "DTypes.h"
#include "DVideoDefines.h"
#include "DVideoFrame.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <functional>

class DRenderQueue {
public:
    explicit DRenderQueue();
    ~DRenderQueue();

public:
    DVoid Start();
    DVoid Stop();
    DVoid Setup(DVoid* wnd, DRect& rect);
    DInt32 GetQueueSize();
    DInt32 PushFrame(DVideoFrame frame);

protected:
    DVoid MessageLoopThread();
    DVoid Notify();
    DVoid ProcessFrame();



private:
    std::deque<DVideoFrame> m_queue;
    std::mutex m_queue_mutex;
    
    std::condition_variable m_cv;
    std::mutex m_cv_mutex;
    std::atomic<bool> m_frame_in_queue{};
    
    std::thread m_thread;
    std::atomic<bool> m_stoped{};

    DVoid* m_context;
    DRect m_destRect;
};

