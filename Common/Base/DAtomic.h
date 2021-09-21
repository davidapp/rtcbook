#pragma once

#include "DConfig.h"


#ifndef D_USE_CPP11_ATOMIC 
    #ifndef D_USE_PLATFORM_ATOMIC
        #define D_USE_CPP11_ATOMIC 1
    #endif
#endif


#if defined(D_USE_CPP11_ATOMIC)

#include <atomic>
typedef std::atomic_bool DAtomBool;
typedef std::atomic_char DAtomChar;
typedef std::atomic_schar DAtomInt8;
typedef std::atomic_uchar DAtomUInt8;
typedef std::atomic_short DAtomInt16;
typedef std::atomic_ushort DAtomUInt16;
typedef std::atomic_int DAtomInt32;
typedef std::atomic_uint DAtomUInt32;
typedef std::atomic_llong DAtomInt64;
typedef std::atomic_ullong DAtomUInt64;
typedef std::atomic_intptr_t DAtomIntPtr;
typedef std::atomic_uintptr_t DAtomUIntPtr;
typedef std::atomic_size_t DAtomSizeT;

typedef std::atomic_flag DAtomFlag;

#define DAtomInc32(addr) (*addr)++
#define DAtomDec32(addr) (*addr)--
#define DAtomAdd32(addr, v)	(*addr).fetch_add(v)
#define DAtomGet32(v) (int)v
#define DAtomSet32(addr, v)	(*addr).store(v)

#else

//Windows
#if defined(BUILD_FOR_WINDOWS)

typedef long int DAtomInt32;
#define DAtomInc32(addr) InterlockedIncrement(addr)
#define DAtomDec32(addr) InterlockedDecrement(addr)
#define DAtomAdd32(addr, v)	InterlockedExchangeAdd(addr, v)
#define DAtomGet32(v) (DInt32)v
#define DAtomSet32(addr, v)	InterlockedExchange(addr, v)

typedef long long int DAtomInt64;
#define DAtomAdd64(addr, v) InterlockedExchangeAdd64(addr, v)
#define DAtomSet64(addr, v)	InterlockedExchange64(addr, v)
#define DAtomAssignPtr(addr, v) InterlockedExchangePointer(addr, v)
#define DAtomCmp(addr, v, comp)	InterlockedCompareExchange(addr, v, comp) //*addr=v (if *addr==comp)
#define DAtomCmp64(addr, v, comp)	InterlockedCompareExchange64(addr, v, comp)
#define DAtomCmpPtr(addr, v, comp)	InterlockedCompareExchangePointer(addr, v, comp)

#define DAtomInc64(addr) InterlockedIncrement64(addr)
#define DAtomDec64(addr) InterlockedDecrement64(addr)
#define DAtomCmpXchg32(addr, ex, comp)	InterlockedCompareExchange(addr, ex, comp) //*addr=ex (if *addr==comp)
#define DAtomCmpXchg64(addr, ex, comp)	InterlockedCompareExchange64(addr, ex, comp)
#define DAtomSwapPtr(a, b, c)   InterlockedCompareExchangePointer(a, b, c)

typedef long int DAtomFlag;

#endif

//iOS
#if defined(BUILD_FOR_IOS)

#include <libkern/OSAtomic.h>
typedef int32_t DAtomInt32;
#define DAtomInc32(addr) OSAtomicIncrement32(addr)
#define DAtomDec32(addr) OSAtomicDecrement32(addr)
#define DAtomAdd32(addr, v) OSAtomicAdd32(v, addr)
#define DAtomGet32(v)  v
#define DAtomSet32(addr, v)  (*addr)=v

#endif

#endif
