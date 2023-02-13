
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

DVoid DRenderQueue::Setup(DUInt32 viewID, DVoid* wnd, DRect& rect)
{
    m_context = wnd;
    m_destRect = rect;
}

DInt32 DRenderQueue::GetQueueSize() 
{
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    DUInt32 sum = 0;
    for (DUInt32 i = 0; i < D_VIEW_COUNT; i++) {
        sum += m_queue[i].size();
    }
    return sum;
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
            for (DUInt32 i = 0; i < D_VIEW_COUNT; i++) {
                if (m_queue[i].size() > 0) {
                    cur_frame = m_queue[i].front();
                    m_queue[i].pop_front();
                    break;
                }
            }
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
    CClientDC dc((HWND)m_context);
    DRect src = DRect(0, 0, f.GetWidth(), f.GetHeight());
    if (f.GetFormat() == DPixelFmt::I420)
    {
        DVideoFrame frameRaw = DVideoFormat::I420ToRAW(f);
        DBITMAPINFOHEADER* pHrd = frameRaw.NewBMPInfoHeader();

        // Double Buffer
        CDC dc_mem;
        dc_mem.CreateCompatibleDC(dc.m_hDC);
        //dc_mem.SetStretchBltMode(HALFTONE);

        HBITMAP bmp_mem = ::CreateCompatibleBitmap(dc.m_hDC, m_destRect.Width(), m_destRect.Height());
        HGDIOBJ bmp_old = ::SelectObject(dc_mem.m_hDC, bmp_mem);
        HBRUSH brush = ::CreateSolidBrush(RGB(100, 100, 100));
        RECT logical_rect = { 0, 0, m_destRect.Width(), m_destRect.Height() };
        ::FillRect(dc_mem, &logical_rect, brush);
        ::DeleteObject(brush);

        dc_mem.StretchDIBits(0, 0, m_destRect.Width(), m_destRect.Height(),
            0, src.Height(), src.Width(), -src.Height(), frameRaw.GetBuf(),
            (const BITMAPINFO*)pHrd, DIB_RGB_COLORS, SRCCOPY);

        dc.BitBlt(m_destRect.left, m_destRect.top, m_destRect.Width(), m_destRect.Height(), dc_mem, 0, 0, SRCCOPY);

        ::SelectObject(dc_mem, bmp_old);
        ::DeleteObject(bmp_mem);
        ::DeleteDC(dc_mem);

        /*dc.StretchDIBits(m_destRect.left, m_destRect.top, m_destRect.Width(), m_destRect.Height(),
            src.left, src.top + src.Height(), src.Width(), -src.Height(), frameRaw.GetBuf(),
            (const BITMAPINFO*)pHrd, DIB_RGB_COLORS, SRCCOPY);*/
        delete pHrd;
    }
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

DInt32 DRenderQueue::PushFrame(DUInt32 viewID, DVideoFrame frame)
{
    {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        m_queue[viewID].push_back(frame);
    }

    Notify();
    return 0;
}

