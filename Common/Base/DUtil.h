﻿#pragma once

#include "DTypes.h"
#include "DAtomic.h"
#include <string>

class DUtil
{
    static std::string UInt64ToStr16(DUInt64 c, DBool bLE = true);
    static std::string BuffToStr(void* p, DUInt32 len);
    D_DISALLOW_ALL_DEFAULT(DUtil)
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
    static DVoid WaitEvent(DEvent& ev, DUInt32 timeinms);

public:
    DVoid* handle;
};

#if defined(BUILD_FOR_WINDOWS)
class DSPinLock
{
public:
    DSPinLock() {
        m_start = 0;
        m_flag = 0;
    }
    inline DVoid Reset() { m_flag = 0; }
    inline DVoid Signal() { m_flag = 1; }
    DUInt32 Wait(DUInt32 need_ms) {
        m_start = GetTickCount();
        while (m_flag != 1) {
            m_now = GetTickCount();
            if (m_now - m_start < need_ms) {
                ::SwitchToThread();
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
private:
    DUInt32 m_start;
    DUInt32 m_now;
    DAtomInt32 m_flag;
};
#else
#endif

class DRWLock
{
public:
    DRWLock();
    ~DRWLock();
    DVoid LockWrite();
    DVoid UnlockWrite();
    DVoid LockRead();
    DVoid UnlockRead();

public:
#if defined(BUILD_FOR_WINDOWS)
    SRWLOCK m_lock;
#else
    pthread_rwlock_t m_lock;
#endif
};

