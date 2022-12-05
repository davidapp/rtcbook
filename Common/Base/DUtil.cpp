#include "DUtil.h"
#include "DXP.h"


///////////////////////////////////////////////////////////////////////////////
// DEvent

#if defined(BUILD_FOR_WINDOWS)
#else
// Linux下，使用 条件变量+Mutex 来模拟 Event
typedef struct tagDEventData
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    DBool flag;
    DWCStr name;
}DEventData;
#endif

DEvent::DEvent()
{
    handle = NULL;
}

DEvent::~DEvent()
{
    Close();
}

DVoid DEvent::Create(DCWStr wName, DBool bAuto)
{
#if defined(BUILD_FOR_WINDOWS)
    handle = CreateEvent(NULL, !bAuto, 0, (LPCWSTR)wName);
#else
    DEventData* event = (DEventData*)malloc(DSizeOf(DEventData));
    event->flag = false;
    pthread_mutex_init(&event->mutex, 0);
    pthread_cond_init(&event->cond, 0);
    event->name = wName;
    handle = event; // handle 指向 DEventData 结构体
#endif
}

DVoid DEvent::Close()
{
#if defined(BUILD_FOR_WINDOWS)
    if (handle)
    {
        CloseHandle(handle);
        handle = NULL;
    }
#else
    if (handle)
    {
        pthread_mutex_destroy(&((DEventData*)handle)->mutex);
        free(handle);
        handle = NULL;
    }
#endif
}

DBool DEvent::Set()
{
#if defined(BUILD_FOR_WINDOWS)
    return ::SetEvent(handle) ? true : false;
#else
    if (handle)
    {
        DEventData* event = (DEventData*)handle;
        pthread_mutex_lock(&event->mutex);
        event->flag = true;
        pthread_cond_signal(&event->cond);  // 激发信号
        pthread_mutex_unlock(&event->mutex);
        return true;
    }
#endif
    return false;
}

DBool DEvent::Reset()
{
#if defined(BUILD_FOR_WINDOWS)
    return ::ResetEvent(handle) ? true : false;
#else
    if (handle)
    {
        DEventData* event = (DEventData*)handle;
        pthread_mutex_lock(&event->mutex);
        event->flag = false;
        pthread_mutex_unlock(&event->mutex);
        return true;
    }
    return false;
#endif
    return false;
}

DBool DEvent::Pulse()
{
#if defined(BUILD_FOR_WINDOWS)
    return ::PulseEvent(handle) ? true : false;
#else
    // 简单模拟
    Set();
    Reset();
    return true;
#endif
}

DVoid DEvent::WaitEvent(DEvent& ev, DUInt32 timeinms)
{
#if defined(BUILD_FOR_WINDOWS)
    ::WaitForSingleObject((HANDLE)ev, timeinms);
#else
    DEventData* event = (DEventData*)ev.handle;
    struct timespec t;
    t.tv_usec = timeinms * 1000; // ms：毫秒；μs：微秒  1ms = 1000μs
    // 1s  = 1000ms
    // 1ms = 1000us
    // 1us = 1000ns
    pthread_mutex_lock(&event->mutex);
    while (!event->flag) {
        // 由于存在虚假唤醒，须使用 while 循环来检查
        pthread_cond_timewait(&event->cond, &event->mutex, &t);
    }
    pthread_mutex_unlock(&event->mutex);
#endif

}


///////////////////////////////////////////////////////////////////////////////
// DSPinLock

DSPinLock::DSPinLock() 
{
    m_start = 0;
    m_flag = 0;
}

DVoid DSPinLock::Reset() 
{ 
    m_flag = 0; 
}

DVoid DSPinLock::Signal()
{ 
    m_flag = 1; 
}

DUInt32 DSPinLock::Wait(DUInt32 need_ms) 
{
    m_start = DXP::GetTickCount32();
    while (m_flag != 1) 
    {
        m_now = DXP::GetTickCount32();
        if (m_now - m_start < need_ms) 
        {
            DXP::SleepSec(0);
        }
        else {
            break;
        }
    }
    DUInt32 diff = m_now - m_start;
    if (diff < need_ms) {
        return diff + 1;
    }
    else {
        return 0;
    }
}


///////////////////////////////////////////////////////////////////////////////
// DRWLock

DRWLock::DRWLock()
{
#if defined(BUILD_FOR_WINDOWS)
    ::InitializeSRWLock(&m_lock);
#else
    pthread_rwlock_init(&m_lock, nullptr);
#endif
}

DRWLock::~DRWLock()
{
#if defined(BUILD_FOR_WINDOWS)
#else
    pthread_rwlock_destroy(&m_lock);
#endif
}

DVoid DRWLock::LockWrite()
{
#if defined(BUILD_FOR_WINDOWS)
    ::AcquireSRWLockExclusive(&m_lock);
#else
    pthread_rwlock_wrlock(&m_lock);
#endif
}

DVoid DRWLock::UnlockWrite()
{
#if defined(BUILD_FOR_WINDOWS)
    ::ReleaseSRWLockExclusive(&m_lock);
#else
    pthread_rwlock_unlock(&m_lock);
#endif
}

DVoid DRWLock::LockRead()
{
#if defined(BUILD_FOR_WINDOWS)
    ::AcquireSRWLockShared(&m_lock);
#else
    pthread_rwlock_rdlock(&m_lock);
#endif
}

DVoid DRWLock::UnlockRead()
{
#if defined(BUILD_FOR_WINDOWS)
    ::ReleaseSRWLockShared(&m_lock);
#else
    pthread_rwlock_unlock(&m_lock);
#endif
}
