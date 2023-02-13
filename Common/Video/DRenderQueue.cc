
#include "DRenderQueue.h"

DRenderQueue::DRenderQueue() 
{
    m_stoped.store(false);
    m_frame_in_queue.store(false);
    m_context = nullptr;
}

DRenderQueue::~DRenderQueue() 
{

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
        
        Render(cur_frame);
    }
    m_frame_in_queue.store(false);
}

#if defined(BUILD_FOR_WINDOWS)
#include <Windows.h>
#include "atlbase.h"
#include "atlapp.h"
#include "atlgdi.h"
#include "DConfig.h"
#include "DTypes.h"
#include "Video/DVideoFormat.h"

DVoid DRenderQueue::Render(DVideoFrame f)
{
    BITMAPINFO* pHeader = (BITMAPINFO*)f.GetUserData();

    CClientDC dc((HWND)m_context);

    DRect src = DRect(0, 0, f.GetWidth(), f.GetHeight());
    /*int crop_width = DMin(src.Width(), m_destRect.Width() * src.Height() / m_destRect.Height());
    int crop_height = DMin(src.Height(), m_destRect.Height() * src.Width() / m_destRect.Width());
    int offset_x = (src.Width() - crop_width) / 2;
    offset_x = int(offset_x / 2) * 2;
    int offset_y = (src.Height() - crop_height) / 2;
    src.left = 0;//offset_x;
    src.right = src.left + crop_width;
    src.top = 0;// offset_y;
    src.bottom = offset_y + crop_height;*/

    if (f.GetFormat() == DPixelFmt::RGB24)
    {
        dc.StretchDIBits(m_destRect.left, m_destRect.top, m_destRect.Width(), m_destRect.Height(), 
            0, 0, f.GetWidth(), f.GetHeight(), f.GetBuf(),
            pHeader, DIB_RGB_COLORS, SRCCOPY);
    }
    else if (f.GetFormat() == DPixelFmt::RAW)
    {
        dc.StretchDIBits(m_destRect.left, m_destRect.top, m_destRect.Width(), m_destRect.Height(), 
            src.left, src.top + src.Height(), src.Width(), -src.Height(), f.GetBuf(),
            pHeader, DIB_RGB_COLORS, SRCCOPY);
    }
    else if (f.GetFormat() == DPixelFmt::I420)
    {
        DVideoFrame frameRaw = DVideoFormat::I420ToRAW(f);
        pHeader->bmiHeader.biBitCount = 24;
        pHeader->bmiHeader.biCompression = BI_RGB;
        pHeader->bmiHeader.biSizeImage = frameRaw.GetSize();
        dc.StretchDIBits(m_destRect.left, m_destRect.top, m_destRect.left + m_destRect.Width(), m_destRect.top + m_destRect.Height(),
            src.left, src.top + src.Height(), src.Width(), -src.Height(), frameRaw.GetBuf(),
            pHeader, DIB_RGB_COLORS, SRCCOPY);
    }

    delete pHeader;
}
#else
DVoid DRenderQueue::Render(DVideoFrame f)
{
    
}
#endif

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

