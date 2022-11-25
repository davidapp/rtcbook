#include "DUtil.h"
#include <locale>
#include "File/DBmpFile.h"
#include "Base/DBuffer.h"
#include "Video/VideoDefines.h"

#if defined(BUILD_FOR_WINDOWS)
#include <dshow.h>
#include "Video/WinDSCamera.h"
#endif

std::string DUtil::BoolToStr(DBool b)
{
    if (b) return "true";
    return "false";
}

std::string DUtil::UInt8ToStr(DUInt8 c)
{
    char buf[10] = {};
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 10, "%u", c);
#else
    sprintf(buf, "%u", c);
#endif
    std::string str = buf;
    return str;
}

std::string DUtil::UInt16ToStr(DUInt16 c, DBool bLE)
{
    char buf[10] = {};
    if (!bLE) {
        c = Swap16(c);
    }
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 10, "%u", c);
#else
    sprintf(buf, "%u", c);
#endif
    std::string str = buf;
    return str;
}

std::string DUtil::UInt32ToStr(DUInt32 c, DBool bLE)
{
    char buf[20] = {};
    if (!bLE) {
        c = Swap32(c);
    }
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 20, "%u", c);
#else
    sprintf(buf, "%u", c);
#endif
    std::string str = buf;
    return str;
}

std::string DUtil::UInt64ToStr(DUInt64 c, DBool bLE)
{
    char buf[40] = {};
    if (!bLE) {
        c = Swap64(c);
    }
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 40, "%llu", c);
#else
    sprintf(buf, "%llu", c);
#endif
    std::string str = buf;
    return str;
}

std::string DUtil::UInt8ToStr16(DUInt8 c)
{
    char buf[10] = {};
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 10, "0x%02x", c);
#else
    sprintf(buf, "0x%02x", c);
#endif
    std::string str = buf;
    return str;
}

std::string DUtil::UInt16ToStr16(DUInt16 c, DBool bLE)
{
    char buf[10] = {};
    if (!bLE) {
        c = Swap16(c);
    }
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 10, "0x%02x %02x", c >> 8, c & 0x00FF);
#else
    sprintf(buf, "0x%02x %02x", c >> 8, c & 0x00FF);
#endif
    std::string str = buf;
    return str;
}

std::string DUtil::UInt32ToStr16(DUInt32 c, DBool bLE)
{
    char buf[20] = {};
    if (!bLE) {
        c = Swap32(c);
    }
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 20, "0x%02x %02x %02x %02x", c >> 24, (c >> 16) & 0x000000FF, (c >> 8) & 0x000000FF, c & 0x000000FF);
#else
    sprintf(buf, "0x%02x %02x %02x %02x", c >> 24, (c >> 16) & 0x000000FF, (c >> 8) & 0x000000FF, c & 0x000000FF);
#endif
    std::string str = buf;
    return str;
}

std::string DUtil::UInt64ToStr16(DUInt64 c, DBool bLE)
{
    char buf[40] = {};
    if (!bLE) {
        c = Swap64(c);
    }
    DBuffer bufTemp(&c, 8);
    std::string str = bufTemp.ToHexString();
    return str;
}

std::string DUtil::Int8ToStr(DInt8 c)
{
    char buf[10] = {};
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 10, "%d", c);
#else
    sprintf(buf, "%d", c);
#endif
    std::string str = buf;
    return str;
}

std::string DUtil::Int16ToStr(DInt16 c, DBool bLE)
{
    char buf[10] = {};
    if (!bLE) {
        c = Swap16(c);
    }
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 10, "%d", c);
#else
    sprintf(buf, "%d", c);
#endif
    std::string str = buf;
    return str;
}

std::string DUtil::Int32ToStr(DInt32 c, DBool bLE)
{
    char buf[20] = {};
    if (!bLE) {
        c = Swap32(c);
    }
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 20, "%d", c);
#else
    sprintf(buf, "%d", c);
#endif
    std::string str = buf;
    return str;
}

std::string DUtil::Int64ToStr(DInt64 c, DBool bLE)
{
    char buf[40] = {};
    if (!bLE) {
        c = Swap64(c);
    }
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 40, "%lld", c);
#else
    sprintf(buf, "%lld", c);
#endif
    std::string str = buf;
    return str;
}

std::string DUtil::AddrToStr(void* p)
{
    char buf[20] = {};
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 20, "0x%p", p);
#else
    sprintf(buf, "0x%p", p);
#endif
    std::string str = buf;
    return str;
}

std::string DUtil::BuffToStr(void* p, DUInt32 len)
{
    DBuffer buf(p, len);
    std::string ret = buf.ToHexString();
    return ret;
}

DInt32 DUtil::StrToInt32(std::string str)
{
    return atoi(str.c_str());
}

DInt32 DUtil::Str16ToInt32(std::wstring wstr)
{
    return atoi(ws2s(wstr).c_str());
}


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
    DEventData* event = (DEventData*)DALLOC(DSizeOf(DEventData));
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
        DFREE(handle);
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
