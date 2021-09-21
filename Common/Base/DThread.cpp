//
//  DThread.cpp
//
//  Created by Dai Wei(bmw.dai@gmail.com) on 03/01/2018 for Dream.
//  Copyright 2018. All rights reserved.
//

#include "DThread.h"
#include "Base/DMemAlloc.h"
#include "Base/DMisc.h"
#include "Base/DTime.h"
#include "Base/DTimer.h"
#include "Base/DString.h"
#include "Base/DStrMgr.h"
#include "Base/DList.h"

#define D_INFINITE 0xFFFFFFFF

#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
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

#if (defined(BUILD_FOR_ANDROID) && (BUILD_FOR_ANDROID==1)) || (defined(BUILD_FOR_LINUX) && (BUILD_FOR_LINUX==1))
#include <pthread.h>
#endif

#if defined(BUILD_FOR_LINUX) && (BUILD_FOR_ANDROID==1)
#include <pthread.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif


DSList g_ThreadData;
DMutex g_tDataMutex(L2W(L"tDataMutex"));


typedef DUInt32(DX86_STDCALL* DCPPThreadStart)(DVoid*);


///////////////////////////////////////////////////////////////////////////////
// DThread
// 
// http://docs.libuv.org/en/v1.x/threading.html?highlight=uv_once#c.uv_once
// 

DBool DThread::Init()
{
    DThreadData* tdata = (DThreadData*)DALLOC(sizeof(DThreadData));
    if (tdata != nullptr)
    {
        tdata->handle = nullptr; // 主线程
        tdata->startTime = DTime::GetUnixTimeStamp64();
        tdata->exitCode = 0;
        tdata->tid = DThread::GetCurThreadID();

        // 添加到链表
        g_tDataMutex.Lock();
        g_ThreadData.Add(tdata, DThread::GetCurThreadID(), DSLinkNodeType::DThreadData);
        g_tDataMutex.Unlock();

        return true;
    }

    return false;
}

// 当前运行线程的ID：Windows上是 DWORD，Linux上是 
// pthread_t 本质上是一个地址 指向 [struct pthread] [线程局部存储] [线程栈]，所以不同进程的可能相同
// syscall(__NR_gettid) 返回的是 LWP(轻量级进程) 的ID，是个int，全局唯一
DUInt32 DThread::GetCurThreadID()
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
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
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    return (DVoid*)::GetCurrentThread(); // 返回一个伪句柄，并非真实的
#else
    return nullptr;
#endif
}

// 创建一个线程，得到其thread_id
DBool DThread::Create(DThreadStart fn, DVoid* para, DUInt32* tid)
{
    return CreateEx(fn, para, 0, 0, false, tid);
}

// stackSize 0表示使用 /STACK:[reserve][,commit] 默认 reserve是1M commit是1页。
// 指定的话，reserve=max(cbStackSize,1M) reserve=cbStackSize
// createFlags 0表示立即调度，CREATE_SUSPENDED表示稍后Resume
DBool DThread::CreateEx(DThreadStart fn, DVoid* para, DUInt32 stackSize, DUInt32 createFlags, DBool bInherit, DUInt32* tid)
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)

    DUInt32 dwThreadID = 0;
    HANDLE hThread = INVALID_HANDLE_VALUE;
    if (!bInherit)
    {
        // Windows上用 _beginthreadex 保证对应的C运行时得到处理
        hThread = (HANDLE)_beginthreadex(NULL, stackSize, (DCPPThreadStart)fn, (void*)para, createFlags, &dwThreadID);
    }
    else
    {
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(sa);          // Used for versioning
        sa.lpSecurityDescriptor = NULL;   // Address of an initialized SD
        sa.bInheritHandle = TRUE;        // 如果想让子进程继承到这个线程句柄，可以设置，通常不需要
        hThread = (HANDLE)_beginthreadex(&sa, stackSize, (DCPPThreadStart)fn, (void*)para, createFlags, &dwThreadID);
    }

    if (hThread != INVALID_HANDLE_VALUE)
    {
        // 返回线程ID
        *tid = dwThreadID;

        // 分配记录节点
        DThreadData* tdata = (DThreadData*)DALLOC(sizeof(DThreadData));
        if (tdata != nullptr)
        {
            tdata->handle = hThread;
            tdata->startTime = DTime::GetUnixTimeStamp64();
            tdata->exitCode = 0;
            tdata->tid = dwThreadID;

            // 分配成功，就添加到链表
            g_tDataMutex.Lock();
            g_ThreadData.Add(tdata, dwThreadID, DSLinkNodeType::DThreadData);
            g_tDataMutex.Unlock();

            return true;
        }
        else
        {
            // 分配失败，有一个未记录信息的野线程
            return false;
        }
    }
#else
    // Linux上用 pthread_create 创建
    pthread_t t_pthread;
    pthread_create(&t_pthread, nullptr, fn, para);
    if (t_pthread != 0)
    {
        DThreadData* tdata = (DThreadData*)DALLOC(sizeof(DThreadData));
        tdata->handle = (void*)t_pthread;
        tdata->startTime = DTime::GetUnixTimeStamp64();
        tdata->exitCode = 0;
#if (defined(BUILD_FOR_IOS) && (BUILD_FOR_IOS==1)) || (defined(BUILD_FOR_MAC) && (BUILD_FOR_MAC==1))
        tdata->tid = pthread_mach_thread_np(t_pthread);
#else
        tdata->tid = t_pthread;
#endif

        g_tDataMutex.Lock();
        g_ThreadData.Add(tdata, (DUInt32)(DUInt64)t_pthread, DSLinkNodeType::DThreadData);
        g_tDataMutex.Unlock();
        *tid = tdata->tid;
        return true;
    }
#endif

    return false;
}

// 关闭线程句柄
DVoid DThread::CloseThreadHandle(DUInt32 tid)
{
    g_tDataMutex.Lock();
    DThreadData* pTData = FindThread(tid);
    if (pTData != NULL)
    {
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
        CloseHandle((HANDLE)(pTData->handle));
#endif
        g_ThreadData.Delete((DVoid*)pTData);
    }
    g_tDataMutex.Unlock();
}

// 挂起一个线程, 返回之前的 suspend count，最大 MAXIMUM_SUSPEND_COUNT = 127
// 挂起之前需要小心，避免被挂起的线程持有lock，造成死锁
DUInt32 DThread::Suspend(DUInt32 tid)
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    DThreadData* pNode = DThread::FindThread(tid);
    if (pNode != NULL)
    {
        return ::SuspendThread(pNode->handle);
    }
#else

#if (defined(BUILD_FOR_IOS) && (BUILD_FOR_IOS==1)) || (defined(BUILD_FOR_MAC) && (BUILD_FOR_MAC==1))
    return thread_suspend(tid);
#else
    return 0;
#endif

#endif
    return 0;
}

// 恢复一个线程
DUInt32 DThread::Resume(DUInt32 tid)
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    DThreadData* pNode = DThread::FindThread(tid);
    if (pNode != NULL)
    {
        return ::ResumeThread(pNode->handle);
    }
    return 0;
#else
#if (defined(BUILD_FOR_IOS) && (BUILD_FOR_IOS==1)) || (defined(BUILD_FOR_MAC) && (BUILD_FOR_MAC==1))
    return thread_resume(tid);
#else
    return 0;
#endif

#endif
}

// 让当前线程切换到其它线程, Windows上每个线程运行20ms左右，GetSystemTimeAdjustment, Spy++
// Sleep(0) 仍然会让当前线程参与调度，Switch则会让低优线程调度。
DBool DThread::SwitchToOthers()
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    return ::SwitchToThread() ? true : false;
#else
    DTimer::SleepSec(0);
    return true;
#endif
}

DVoid DThread::Terminate(DUInt32 tid, DUInt32 exitcode)
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    DThreadData* pNode = DThread::FindThread(tid);
    if (pNode != NULL)
    {
        ::TerminateThread((HANDLE)(pNode->handle), (DWORD)exitcode);
    }
#else

#endif
}

DBool DThread::IsActive(DUInt32 tid)
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    DThreadData* pNode = DThread::FindThread(tid);
    if (pNode != NULL)
    {
        DWORD dwCode = 0;
        ::GetExitCodeThread((HANDLE)(pNode->handle), &dwCode);
        if (dwCode == STILL_ACTIVE) // 0x103
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
#else
    return false;
#endif
}

DVoid DThread::SleepMs(DUInt32 ms)
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    ::Sleep(ms);
#else

#endif
}

#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
DUInt64 FileTimeToDUInt64(PFILETIME pft)
{
    return(Int64ShllMod32(pft->dwHighDateTime, 32) | pft->dwLowDateTime);
}
#endif

DBool DThread::GetRunTimes(DUInt32 tid, DUInt64* kernel_time, DUInt64* user_time)
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    DThreadData* pNode = DThread::FindThread(tid);
    if (pNode)
    {
        FILETIME ftKernelTime;
        FILETIME ftUserTime;
        FILETIME ftDummy;
        ::GetThreadTimes(GetCurrentThread(), &ftDummy, &ftDummy, &ftKernelTime, &ftUserTime);
        *kernel_time = FileTimeToDUInt64(&ftKernelTime);
        *user_time = FileTimeToDUInt64(&ftUserTime);
        return true;
    }
    return false;
#else
    return false;
#endif
}

DStringA DThread::GetContextStr(DUInt32 tid)
{
    DStringA strRet;
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    DThreadData* pNode = DThread::FindThread(tid);
    if (pNode)
    {
        CONTEXT Context = {};
        Context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
        GetThreadContext(pNode->handle, &Context);

#if defined(BUILD_FOR_X86) && (BUILD_FOR_X86==1)
        strRet = DThread::WinX86ContextToString(&Context);
#elif defined(BUILD_FOR_X64) && (BUILD_FOR_X64==1)
        strRet = DThread::WinX64ContextToString(&Context);
#endif
    }
    return strRet;
#else
    return strRet;
#endif

}

DIntArray DThread::GetAllThreadID()
{
    DIntArray arrRet;
    DSLinkNode* pNode = g_ThreadData.GetHead();
    while (pNode != nullptr)
    {
        arrRet.Add(pNode->nData32);
        pNode = pNode->pNext;
    }
    return arrRet;
}

#if defined(BUILD_FOR_X86) && (BUILD_FOR_X86==1)
DStringA DThread::WinX86ContextToString(PCONTEXT pc)
{
    DStringA strRet, strTemp;

    // CONTEXT_DEBUG_REGISTERS
    strTemp.Format("Dr0=0x%08x\r\n", pc->Dr0);
    strRet += strTemp;
    strTemp.Format("Dr1=0x%08x\r\n", pc->Dr1);
    strRet += strTemp;
    strTemp.Format("Dr2=0x%08x\r\n", pc->Dr2);
    strRet += strTemp;
    strTemp.Format("Dr3=0x%08x\r\n", pc->Dr3);
    strRet += strTemp;
    strTemp.Format("Dr6=0x%08x\r\n", pc->Dr6);
    strRet += strTemp;
    strTemp.Format("Dr7=0x%08x\r\n", pc->Dr7);
    strRet += strTemp;

    // CONTEXT_SEGMENTS
    strTemp.Format("SegGs=0x%08x\r\n", pc->SegGs);
    strRet += strTemp;
    strTemp.Format("SegFs=0x%08x\r\n", pc->SegFs);
    strRet += strTemp;
    strTemp.Format("SegEs=0x%08x\r\n", pc->SegEs);
    strRet += strTemp;
    strTemp.Format("SegDs=0x%08x\r\n", pc->SegDs);
    strRet += strTemp;

    // CONTEXT_INTEGER
    strTemp.Format("Edi=0x%08x\r\n", pc->Edi);
    strRet += strTemp;
    strTemp.Format("Esi=0x%08x\r\n", pc->Esi);
    strRet += strTemp;
    strTemp.Format("Ebx=0x%08x\r\n", pc->Ebx);
    strRet += strTemp;
    strTemp.Format("Edx=0x%08x\r\n", pc->Edx);
    strRet += strTemp;
    strTemp.Format("Ecx=0x%08x\r\n", pc->Ecx);
    strRet += strTemp;
    strTemp.Format("Eax=0x%08x\r\n", pc->Eax);
    strRet += strTemp;

    // CONTEXT_CONTROL
    strTemp.Format("Ebp=0x%08x\r\n", pc->Ebp);
    strRet += strTemp;
    strTemp.Format("Eip=0x%08x\r\n", pc->Eip);
    strRet += strTemp;
    strTemp.Format("SegCs=0x%08x\r\n", pc->SegCs);
    strRet += strTemp;
    strTemp.Format("EFlags=0x%08x\r\n", pc->EFlags);
    strRet += strTemp;
    strTemp.Format("Esp=0x%08x\r\n", pc->Esp);
    strRet += strTemp;
    strTemp.Format("SegSs=0x%08x\r\n", pc->SegSs);
    strRet += strTemp;

    return strRet;
}
#endif
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
#if defined(BUILD_FOR_X64) && (BUILD_FOR_X64==1)
DStringA DThread::WinX64ContextToString(PCONTEXT pc)
{
    DStringA strRet, strTemp;
    return strRet;
}
#endif
#endif

// 等待某个其它线程运行结束，或者超时
DVoid DThread::WaitFinish(DUInt32 tid, DUInt32 timeinms)
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    DThreadData* pNode = DThread::FindThread(tid);
    if (pNode != NULL)
    {
        ::WaitForSingleObject(pNode->handle, INFINITE);
    }
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
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    WaitForSingleObject((HANDLE)ev, INFINITE);
#else
    DEventData* event = (DEventData*)ev.handle;
    pthread_mutex_lock(&event->mutex);
    while (!event->flag)
        pthread_cond_wait(&event->cond, &event->mutex);
    pthread_mutex_unlock(&event->mutex);
#endif

}


DThreadData* DThread::FindThread(DUInt32 tid)
{
    g_tDataMutex.Lock();
    DSLinkNode* pNode = g_ThreadData.Find32(tid);
    g_tDataMutex.Unlock();
    if (pNode != nullptr)
    {
        DThreadData* pTData = (DThreadData*)(pNode->pData);
        return pTData;
    }
    return nullptr;
}

DStringA DThread::GetDumpString(DThreadData* tdata)
{
    DStringA strRet;
    return strRet;
}

DStringA DThread::GetAllDumpString()
{
    DStringA strRet, strTemp;
    g_tDataMutex.Lock();

    DSLinkNode* pNode = g_ThreadData.GetHead();
    while (pNode != nullptr)
    {
        DThreadData* pTD = (DThreadData*)(pNode->pData);
        DUInt32 tid = pNode->nData32;
        if (pTD->handle == NULL)
        {
            strTemp.Format("tid:%u, main\r\n", tid);
        }
        else
        {
            strTemp.Format("tid:%u, %u\r\n", tid, pTD->handle);
        }
        strRet += strTemp;
        pNode = pNode->pNext;
    }

    g_tDataMutex.Unlock();
    return strRet;
}

DVoid DThread::Dump()
{
    DStringA strRet = GetAllDumpString();
    strRet.Print();
}


///////////////////////////////////////////////////////////////////////////////
// DMutex

DMutex::DMutex()
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    handle = ::CreateMutex(NULL, FALSE, (LPCWSTR)DMisc::RandomStr(16).ToUnicode().GetStr());
#else
    pthread_mutex_t* dMutex = new pthread_mutex_t();
    pthread_mutex_init(dMutex, NULL);
    handle = dMutex;
#endif
}

DMutex::DMutex(DCWStr strName)
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    handle = ::CreateMutex(NULL, FALSE, (LPCWSTR)strName);
#else
    pthread_mutex_t* dMutex = new pthread_mutex_t();
    pthread_mutex_init(dMutex, NULL);
    handle = dMutex;
#endif
}

DMutex::~DMutex()
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    if (handle)
    {
        CloseHandle(handle);
        handle = NULL;
    }
#else
    if (handle)
    {
        pthread_mutex_destroy((pthread_mutex_t*)handle);
        delete (pthread_mutex_t*)handle;
        handle = NULL;
    }
#endif
}

void DMutex::Lock()
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    if (handle)
    {
        ::WaitForSingleObject(handle, INFINITE);
    }
#else
    if (handle)
    {
        pthread_mutex_lock((pthread_mutex_t*)handle);
    }
#endif
}

void DMutex::Unlock()
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    if (handle)
    {
        ReleaseMutex(handle);
    }
#else
    if (handle)
    {
        pthread_mutex_unlock((pthread_mutex_t*)handle);
    }
#endif
}



///////////////////////////////////////////////////////////////////////////////
// DEvent

DEvent::DEvent()
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    handle = CreateEvent(NULL, TRUE, 0, NULL);
#else
    DEventData* event = (DEventData*)DALLOC(sizeof(DEventData));
    event->flag = false;
    pthread_mutex_init(&event->mutex, 0);
    pthread_cond_init(&event->cond, 0);
    handle = event;
#endif
}

DEvent::DEvent(DCWStr wName, DBool bAuto)
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    handle = CreateEvent(NULL, !bAuto, 0, (LPCWSTR)wName);
#else
    DEventData* event = (DEventData*)DALLOC(sizeof(DEventData));
    event->flag = false;
    pthread_mutex_init(&event->mutex, 0);
    pthread_cond_init(&event->cond, 0);
    event->name = wName;
    handle = event;
#endif
}

DEvent::~DEvent()
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
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
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    return ::SetEvent(handle) ? true : false;
#else
    if (handle)
    {
        DEventData* event = (DEventData*)handle;
        pthread_mutex_lock(&event->mutex);
        event->flag = true;
        pthread_cond_signal(&event->cond);
        pthread_mutex_unlock(&event->mutex);
        return true;
    }
#endif
    return false;
}

DBool DEvent::Reset()
{
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
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
#if defined(BUILD_FOR_WINDOWS) && (BUILD_FOR_WINDOWS==1)
    return ::PulseEvent(handle) ? true : false;
#else
    return false;
#endif
}




///////////////////////////////////////////////////////////////////////////////
// DSem



///////////////////////////////////////////////////////////////////////////////
// DCond




///////////////////////////////////////////////////////////////////////////////
// DRWLock

