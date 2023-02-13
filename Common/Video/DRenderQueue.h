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

#define D_LOCAL_VIEW 0
#define D_REMOTE_VIEW 1
#define D_VIEW_COUNT 2

class DRenderQueue {
public:
    explicit DRenderQueue();
    ~DRenderQueue();

public:
    DVoid Start();
    DVoid Stop();
    DVoid Setup(DUInt32 viewID, DVoid* wnd, DRect& rect);
    DInt32 GetQueueSize();
    DInt32 PushFrame(DUInt32 viewID, DVideoFrame frame);

protected:
    DVoid MessageLoopThread();
    DVoid Notify();
    DVoid ProcessFrame();

    DVoid Render(DVideoFrame f);

private:
    std::deque<DVideoFrame> m_queue[D_VIEW_COUNT];
    std::mutex m_queue_mutex;
    
    std::condition_variable m_cv;
    std::mutex m_cv_mutex;
    std::atomic<bool> m_frame_in_queue{};
    
    std::thread m_thread;
    std::atomic<bool> m_stoped{};
    
    typedef std::pair<DVoid*, DRect> DConfig;
    //DVoid* m_context;
    //DRect m_destRect;
};

