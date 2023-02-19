#pragma once

#include "DTypes.h"
#include "DVideoDefines.h"
#include "DVideoFrame.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <vector>
#include <thread>
#include <functional>

enum DViewID {
    D_LOCAL_VIEW = 0,
    D_REMOTE_VIEW = 1,
    D_VIEW_COUNT
};

typedef struct tagDRenderConfig {
    DVoid* context;
    DRect destRect;
    DBool mirrored;
    DUInt32 fps;
    DUInt64 lastPTS;
    DScaleMode renderMode;
}DRenderConfig;

class DRenderQueue {
public:
    explicit DRenderQueue();
    ~DRenderQueue();

public:
    DVoid Start();
    DVoid Stop();
    DVoid Setup(DUInt32 viewID, DVoid* wnd, DRect& rect);
    DVoid SetMirror(DUInt32 viewID, DBool bMirror);
    DVoid SetFPS(DUInt32 viewID, DUInt32 fps);
    DVoid SetRenderMode(DUInt32 viewID, DScaleMode mode);

public:
    DInt32 GetQueueSize();
    DInt32 PushFrame(DUInt32 viewID, DVideoFrame frame);

protected:
    DVoid MessageLoopThread();
    DVoid Notify();
    DVoid ProcessFrame();

    DVoid Render(DUInt32 viewID, DVideoFrame f);

private:
    std::deque<DVideoFrame> m_queue[D_VIEW_COUNT];
    std::mutex m_queue_mutex;
    
    std::condition_variable m_cv;
    std::mutex m_cv_mutex;
    std::atomic<bool> m_frame_in_queue{};
    
    std::thread m_thread;
    std::atomic<bool> m_stoped{};
    
    std::vector<DRenderConfig> m_configs;
};

