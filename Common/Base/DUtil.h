#pragma once

#include "DTypes.h"
#include "DAtomic.h"
#include <string>

class DUtil
{
public:
    static std::string ws2s(const std::wstring& ws);
    static std::wstring s2ws(const std::string& s);
    static std::string& replace_str(std::string& str, const std::string& to_replaced, const std::string& newchars);

public:
    static DInt32 isatof(DInt32 c);
    static DInt32 isAtoF(DInt32 c);

public:
    static DUInt16 Swap16(DUInt16 h);
    static DUInt32 Swap32(DUInt32 h);
    static DUInt64 Swap64(DUInt64 h);

public:
    static std::string BoolToStr(DBool b);
    static std::string UInt8ToStr(DUInt8 c);
    static std::string UInt16ToStr(DUInt16 c, DBool bLE = true);
    static std::string UInt32ToStr(DUInt32 c, DBool bLE = true);
    static std::string UInt64ToStr(DUInt64 c, DBool bLE = true);
    static std::string UInt8ToStr16(DUInt8 c);
    static std::string UInt16ToStr16(DUInt16 c, DBool bLE = true);
    static std::string UInt32ToStr16(DUInt32 c, DBool bLE = true);
    static std::string UInt64ToStr16(DUInt64 c, DBool bLE = true);
    static std::string Int8ToStr(DInt8 c);
    static std::string Int16ToStr(DInt16 c, DBool bLE = true);
    static std::string Int32ToStr(DInt32 c, DBool bLE = true);
    static std::string Int64ToStr(DInt64 c, DBool bLE = true);
    static std::string AddrToStr(void* p);
    static std::string BuffToStr(void* p, DUInt32 len);
    static DInt32 StrToInt32(std::string str);
    static DInt32 Str16ToInt32(std::wstring wstr);

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

