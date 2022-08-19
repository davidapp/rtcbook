#include "DThread.h"

#if defined(BUILD_FOR_WINDOWS)
#include <process.h> //for _beginthreadex
#else
#include <errno.h>
#include <pthread.h> //for pthread
#include <unistd.h>
#endif

#if (defined(BUILD_FOR_IOS) && (BUILD_FOR_IOS==1)) || (defined(BUILD_FOR_MAC) && (BUILD_FOR_MAC==1))
#include <mach/mach.h>
#include <mach/task.h>
#endif

#if defined(BUILD_FOR_LINUX) && (BUILD_FOR_ANDROID==1)
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif


DBool DThread::Create(DThreadStart fn, DVoid* para, DUInt32* tid, DVoid** handle)
{
    return CreateEx(fn, para, 0, 0, false, tid, handle);
}

DBool DThread::CreateEx(DThreadStart fn, DVoid* para, DUInt32 stackSize, DUInt32 createFlags, DBool bInherit, DUInt32* tid, DVoid** handle)
{
#if defined(BUILD_FOR_WINDOWS)
    DWORD dwThreadID = 0;
    HANDLE hThread = INVALID_HANDLE_VALUE;
    if (!bInherit)
    {
        hThread = (HANDLE)_beginthreadex(NULL, stackSize, fn, (void*)para, createFlags, (unsigned*)&dwThreadID);
    }
    else
    {
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(sa);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = TRUE;
        hThread = (HANDLE)_beginthreadex(&sa, stackSize, fn, (void*)para, createFlags, (unsigned*)&dwThreadID);
    }

    if (hThread == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    *tid = dwThreadID;
    *handle = hThread;

#else

    // Linux上用 pthread_create 创建
    pthread_t t_pthread;
    // 参数1 是个输出参数，若成功，会返回 thread id
    // 参数2 NULL 表示默认的属性
    // 参数3 线程函数，必须是 __cdecl 的，Windows下为 __stdcall
    // 参数4 void*类型的参数
    int ret = pthread_create(&t_pthread, NULL, fn, para);
    if (ret != 0) return false; // 常见错误码有 EAGAIN EINVAL

    tdata->handle = (void*)t_pthread;
    tdata->startTime = DTime::GetUnixTimeStamp64();
    tdata->exitCode = 0;
#if (defined(BUILD_FOR_IOS) && (BUILD_FOR_IOS==1)) || (defined(BUILD_FOR_MAC) && (BUILD_FOR_MAC==1))
    tdata->tid = pthread_mach_thread_np(t_pthread);
#else
    tdata->tid = t_pthread;
#endif
    // 添加到链表中
    g_tDataMutex.Lock();
    g_ThreadData.Add(tdata, (DUInt32)(DUInt64)t_pthread, DSLinkNodeType::DThreadData);
    g_tDataMutex.Unlock();

    // 返回线程ID
    *tid = tdata->tid;


#endif

    return true;
}

DVoid DThread::CloseThreadHandle(DVoid* handle)
{
#if defined(BUILD_FOR_WINDOWS)
    CloseHandle((HANDLE)handle);
#else
    D_UNUSED(handle)
#endif
}

DUInt32 DThread::GetCurThreadID()
{
#if defined(BUILD_FOR_WINDOWS)
    return ::GetCurrentThreadId();
#elif defined(BUILD_FOR_IOS) && (BUILD_FOR_IOS==1)
    return mach_thread_self();
#elif defined(BUILD_FOR_MAC) && (BUILD_FOR_MAC==1)
    return mach_thread_self();
#elif defined(BUILD_FOR_ANDROID) && (BUILD_FOR_ANDROID==1)
    return (DUInt32)gettid();
#elif defined(BUILD_FOR_LINUX) && (BUILD_FOR_LINUX==1)
    return (DUInt32)syscall(SYS_gettid);
#endif
}

DVoid* DThread::GetCurThreadHandle()
{
#if defined(BUILD_FOR_WINDOWS)
    return (DVoid*)::GetCurrentThread(); // fake handle
#else
    return nullptr;
#endif
}

DVoid DThread::Terminate(DVoid* handle, DUInt32 exitcode)
{
#if defined(BUILD_FOR_WINDOWS)
    ::TerminateThread((HANDLE)handle, (DWORD)exitcode);
#else

#endif
}

DBool DThread::IsActive(DVoid* handle)
{
#if defined(BUILD_FOR_WINDOWS)
    DWORD dwCode = 0;
    ::GetExitCodeThread((HANDLE)handle, &dwCode);
    if (dwCode == STILL_ACTIVE) // 0x103
    {
        return true;
    }
    else
    {
        return false;
    }
#else
    return false;
#endif
}


DUInt32 DThread::Suspend(DVoid* handle)
{
#if defined(BUILD_FOR_WINDOWS)
    return ::SuspendThread(handle);
#else
#if (defined(BUILD_FOR_IOS) && (BUILD_FOR_IOS==1)) || (defined(BUILD_FOR_MAC) && (BUILD_FOR_MAC==1))
    return thread_suspend(handle);
#else
    return 0;
#endif

#endif
    return 0;
}

// 恢复一个线程
DUInt32 DThread::Resume(DVoid* handle)
{
#if defined(BUILD_FOR_WINDOWS)
    return ::ResumeThread(handle);
#else
#if (defined(BUILD_FOR_IOS) && (BUILD_FOR_IOS==1)) || (defined(BUILD_FOR_MAC) && (BUILD_FOR_MAC==1))
    return thread_resume(tid);
#else
    return 0;
#endif

#endif
}


DBool DThread::SwitchToOthers()
{
#if defined(BUILD_FOR_WINDOWS)
    return ::SwitchToThread() ? true : false;
#else
    DTimer::SleepSec(0);
    return true;
#endif
}


DVoid DThread::SleepMs(DUInt32 ms)
{
#if defined(BUILD_FOR_WINDOWS)
    ::Sleep(ms);
#else

#endif
}

#if defined(BUILD_FOR_WINDOWS)
DUInt64 FileTimeToDUInt64(PFILETIME pft)
{
    return(Int64ShllMod32(pft->dwHighDateTime, 32) | pft->dwLowDateTime);
}
#endif


// 等待某个其它线程运行结束，或者超时
DVoid DThread::WaitFinish(DVoid* handle, DUInt32 timeinms)
{
#if defined(BUILD_FOR_WINDOWS)
    ::WaitForSingleObject(handle, INFINITE);
#else
    DThreadData* pNode = DThread::FindThread(tid);
    if (pNode != nullptr)
    {
#if (defined(BUILD_FOR_IOS) && (BUILD_FOR_IOS==1)) || (defined(BUILD_FOR_MAC) && (BUILD_FOR_MAC==1))
        pthread_join(pthread_from_mach_thread_np(tid), nullptr);
#else
        pthread_join(tid, nullptr);
#endif
    }
#endif
}

// 等待某个事件对象被触发，或者超时
DVoid DThread::WaitEvent(DEvent& ev, DUInt32 timeinms)
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

DBool DThread::IsObjSignaled(DVoid* handle)
{
    if (::WaitForSingleObject(handle, 0) == WAIT_OBJECT_0) {
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// DMutex

#if defined(BUILD_FOR_WINDOWS)
#else
// Linux下，使用 条件变量+Mutex 来模拟 Event
typedef struct tagDEventData
{
    pthread_mutex_t mutex;
    pthread_mutexattr_t attr;
    DInt32 type;
    DWCStr name;
}DMutexData;
#endif

DMutex::DMutex()
{
    handle = NULL;
}

DMutex::~DMutex()
{
    Close();
}

DVoid DMutex::Create(DCWStr strName, DInt32 type)
{
#if defined(BUILD_FOR_WINDOWS)
    handle = ::CreateMutex(NULL, FALSE, (LPCWSTR)strName);
#else
    DMutexData* dMutex = (pthread_mutex_t*)DALLOC(DSizeOf(DMutexData));
    dMutex->name = strName;
    dMutex->type = type;
    if (type == PTHREAD_MUTEX_NORMAL) {
        pthread_mutex_init(&dMutex->mutex, NULL);
    }
    else if (type == PTHREAD_MUTEX_ERRORCHECK) {
        pthread_mutexattr_init(&dMutex->attr);
        pthread_mutexattr_settype(&dMutex->attr, PTHREAD_MUTEX_ERRORCHECK);
        pthread_mutex_init(&dMutex->mutex, &dMutex->attr);
    }
    else if (type == PTHREAD_MUTEX_RECURSIVE) {
        pthread_mutexattr_init(&dMutex->attr);
        pthread_mutexattr_settype(&dMutex->attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&dMutex->mutex, &dMutex->attr);
    }
    handle = dMutex;
#endif
}

DVoid DMutex::Close()
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
        DInt32 type = &dMutex->type;
        if (type == PTHREAD_MUTEX_ERRORCHECK || type == PTHREAD_MUTEX_RECURSIVE) {
            pthread_mutexattr_destroy(&dMutex->attr);
        }
        pthread_mutex_destroy((pthread_mutex_t*)handle->mutex);
        DFREE(handle);
        handle = NULL;
    }
#endif
}

DVoid DMutex::Lock()
{
#if defined(BUILD_FOR_WINDOWS)
    if (handle)
    {
        ::WaitForSingleObject(handle, INFINITE);
    }
#else
    if (handle)
    {
        pthread_mutex_lock((pthread_mutex_t*)handle->mutex);
    }
#endif
}

DVoid DMutex::Unlock()
{
#if defined(BUILD_FOR_WINDOWS)
    if (handle)
    {
        ReleaseMutex(handle);
    }
#else
    if (handle)
    {
        pthread_mutex_unlock((pthread_mutex_t*)handle->mutex);
    }
#endif
}

DVoid DMutex::TryLock()
{
#if defined(BUILD_FOR_WINDOWS)
    if (handle)
    {
        //TryLockMutex(handle);
    }
#else
    if (handle)
    {
        pthread_mutex_trylock((pthread_mutex_t*)handle->mutex);
    }
#endif
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


///////////////////////////////////////////////////////////////////////////////
// DSem



///////////////////////////////////////////////////////////////////////////////
// DRWLock

#if defined(BUILD_FOR_WINDOWS)

DVoid DRWLock::Init()
{
    ::InitializeSRWLock(&m_lock);
}

DVoid DRWLock::AcquireExclusive()
{
    ::AcquireSRWLockExclusive(&m_lock);
}

DVoid DRWLock::ReleaseExclusive()
{
    ::ReleaseSRWLockExclusive(&m_lock);
}

DVoid DRWLock::AcquireShared()
{
    ::AcquireSRWLockShared(&m_lock);
}

DVoid DRWLock::ReleaseShared()
{
    ::ReleaseSRWLockShared(&m_lock);
}

#else



#endif


///////////////////////////////////////////////////////////////////////////////
// DCondVar

DCondVar::DCondVar()
{
    handle = nullptr;
}

DCondVar::~DCondVar()
{
    Close();
}

DVoid DCondVar::Create()
{
#if defined(BUILD_FOR_WINDOWS)
    PCONDITION_VARIABLE condVar = (PCONDITION_VARIABLE)malloc(DSizeOf(CONDITION_VARIABLE));
    ::InitializeConditionVariable(&m_cond); // user-object
    handle = condVar;
#else
    pthread_cond_t* condVar = (pthread_cond_t*)DALLOC(DSizeOf(pthread_cond_t));
    pthread_cond_init(condVar, NULL);
    handle = condVar;
#endif
}

DVoid DCondVar::Close()
{
#if defined(BUILD_FOR_WINDOWS)

#else
    pthread_cond_destroy((pthread_cond_t*)handle);
    DFREE(handle);
    handle = nullptr;
#endif
}

DBool DCondVar::SleepSRW(DRWLock lock, DUInt32 dwMilliseconds, ULONG Flags)
{
#if defined(BUILD_FOR_WINDOWS)
    return ::SleepConditionVariableSRW(&m_cond, &(lock.m_lock), dwMilliseconds, Flags);
#else

#endif
}

DVoid DCondVar::Wake()
{
#if defined(BUILD_FOR_WINDOWS)

#else
    pthread_cond_signal(&m_cond);
#endif
}

DVoid DCondVar::WakeAll()
{
#if defined(BUILD_FOR_WINDOWS)

#else
    pthread_cond_broadcast(&m_cond);
#endif
}
