#pragma once

#include "DTypes.h"

typedef DVoid* (*DThreadStart)(DVoid* para);

typedef struct tagDThreadData
{
    DVoid* handle; // 线程的句柄: Windows上是 HANDLE，Linux上是 pthread_t
    DUInt32 tid;	// 线程的ID：Windows上是 DWORD，Linux上是 LWP ID 
    DVoid* fn;		// 线程启动函数
    DVoid* para;	// 线程启动参数
    DUInt64 startTime;	// 开始时间
    DUInt32 stackSize;	// 线程栈大小
    DVoid* exitCode;	// 线程退出码
    DBool   isMain;		// 是否是主线程
} DThreadData;

class DEvent;

class DThread
{
public:
    // 创建与销毁
    static DBool   Init();
    static DBool   Create(DThreadStart fn, DVoid* para, DUInt32* tid);
    static DBool   CreateEx(DThreadStart fn, DVoid* para, DUInt32 stackSize, DUInt32 createFlags, DBool bInherit, DUInt32* tid);
    static DVoid   CloseThreadHandle(DUInt32 tid);

    static DUInt32 GetCurThreadID();
    static DVoid* GetCurThreadHandle();

    static DVoid   Terminate(DUInt32 tid, DUInt32 exitcode);
    static DBool   IsActive(DUInt32 tid);

    static DBool   SwitchToOthers();
    static DUInt32 Suspend(DUInt32 tid);
    static DUInt32 Resume(DUInt32 tid);
    static DVoid   SleepMs(DUInt32 ms);

    static DBool   GetRunTimes(DUInt32 tid, DUInt64* kernel_time, DUInt64* user_time);

public:
    static DVoid WaitFinish(DUInt32 tid, DUInt32 timeinms);
    static DVoid WaitEvent(DEvent& ev, DUInt32 timeinms);
    
    D_DISALLOW_ALL_DEFAULT(DThread)
};


// 互斥量
class DMutex
{
public:
    DMutex();
    explicit DMutex(DCWStr strName);
    ~DMutex();
    void Lock();
    void Unlock();
private:
    DVoid* handle;
};


#if defined(BUILD_FOR_WINDOWS)
#else
#include <pthread.h> //for pthread
#include <unistd.h>
typedef struct tagDEventData
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    DBool flag;
    DCWStr name;
}DEventData;
#endif

// 事件同步
class DEvent
{
public:
    DEvent();
    explicit DEvent(DCWStr wName, DBool bAuto = false);
    ~DEvent();
    DBool Set();
    DBool Reset();
    DBool Pulse();
    operator DVoid* () { return handle; }

public:
    DVoid* handle;
};


// 信号量
class DSem
{
public:


};

// 条件变量
class DCond
{
public:


};

// 读写锁
class DRWLock
{
public:


};
