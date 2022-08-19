#pragma once

#include "DTypes.h"
#include <string>
#include <vector>

//typedef DVoid* (__cdecl* DThreadStart)(DVoid* para);
typedef unsigned(__stdcall* _beginthreadex_proc_type)(void*);
typedef DUInt32(__stdcall* DThreadStart)(DVoid*);

class DEvent;

class DThread
{
public:
    static DBool   Create(DThreadStart fn, DVoid* para, DUInt32* tid, DVoid** handle);
    static DBool   CreateEx(DThreadStart fn, DVoid* para, DUInt32 stackSize, DUInt32 createFlags, DBool bInherit, DUInt32* tid, DVoid** handle);
    static DVoid   CloseThreadHandle(DVoid* handle);

    static DUInt32 GetCurThreadID();
    static DVoid*  GetCurThreadHandle();

    static DVoid   Terminate(DVoid* handle, DUInt32 exitcode);
    static DBool   IsActive(DVoid* handle);

    static DBool   SwitchToOthers();
    static DUInt32 Suspend(DVoid* handle);
    static DUInt32 Resume(DVoid* handle);
    static DVoid   SleepMs(DUInt32 ms);

public:
    static DVoid WaitFinish(DVoid* handle, DUInt32 timeinms);
    static DVoid WaitEvent(DEvent& ev, DUInt32 timeinms);
    static DBool IsObjSignaled(DVoid* handle);

public:
    D_DISALLOW_ALL_DEFAULT(DThread)
};


class DMutex
{
public:
    DMutex();
    ~DMutex();
    DVoid Create(DCWStr strName = NULL, DInt32 type = 0);
    DVoid Close();
    DVoid Lock();
    DVoid Unlock();
    DVoid TryLock();
public:
    DVoid* handle;
};


class DEvent
{
public:
    DEvent();
    ~DEvent();
    DVoid Create(DCWStr wName, DBool bAuto = false);
    DVoid Close();
    DBool Set();
    DBool Reset();
    DBool Pulse();
    operator DVoid* () { return handle; }

public:
    DVoid* handle;
};



class DSem
{
public:
    DSem();
    ~DSem();

public:
    DVoid* handle;
};


class DRWLock
{
public:
    DVoid Init();
    DVoid AcquireExclusive();
    DVoid ReleaseExclusive();
    DVoid AcquireShared();
    DVoid ReleaseShared();

public:
#if defined(BUILD_FOR_WINDOWS)
    SRWLOCK m_lock;
#else
    pthread_cond_t m_cond;
#endif
};


class DCondVar
{
public:
    DCondVar();
    ~DCondVar();
    DVoid Create();
    DVoid Close();
    DVoid Wake();
    DVoid WakeAll();

public:
    DVoid* handle;
#if defined(BUILD_FOR_WINDOWS)
    DBool SleepSRW(DRWLock lock, DUInt32 dwMilliseconds, ULONG Flags);
    CONDITION_VARIABLE m_cond; // user-mode object
#else
    DVoid WaitMutex(DMutex mutex);
    DVoid WaitMutex(DMutex mutex, DUInt32 time);
    pthread_cond_t m_cond;
#endif
};
