#pragma once

#include "DConfig.h"

//Basic Types
typedef unsigned char DByte;
typedef char DChar;
typedef char* DStr;
typedef const char* DCStr;

#if defined(BUILD_FOR_WINDOWS)
//for L"xxx" on Windows
typedef wchar_t DWChar;
typedef wchar_t* DWStr;
typedef const wchar_t* DCWStr;
#else
typedef unsigned short DWChar;
typedef unsigned short* DWStr;
typedef const unsigned short* DCWStr;
#endif

typedef unsigned int DUChar;
typedef unsigned int* DUStr;
typedef const unsigned int* DCUStr;

typedef char DInt8;
typedef unsigned char DUInt8;
typedef short DInt16;
typedef unsigned short DUInt16;
typedef int DInt32;
typedef unsigned int DUInt32;
typedef long long DInt64;
typedef unsigned long long DUInt64;
typedef DUInt64 DPtrDiff;

typedef float DFloat;
typedef double DDouble;
typedef long double DLDouble;
typedef bool  DBool;
typedef void  DVoid;
typedef void* DVoidPtr;

#if defined(TARGET_64BIT)
typedef DUInt64 DSizeT;
typedef DUInt64 DHandle;
typedef DUInt64 DULong;
#else 
typedef DUInt32 DSizeT;
typedef DUInt32 DHandle;
typedef DUInt32 DULong;
#endif


//Utility
#define DMax(x,y) ((x>y)?x:y)
#define DMin(x,y) ((x<y)?x:y)
#define D_INFINITE 0xFFFFFFFF
#define D_UNUSED(v) (void)(v)
#define D_LengthOf(x) (sizeof(x)/sizeof(*x))
#define D_DIM_OF(Array) (sizeof(Array)/sizeof(Array[0]))
#define D_INRANGE(low, Num, High) (((low) <= (Num)) && ((Num) <= (High)))
#define D_CLAMP(x,a,b) (DMin(b,DMax(a,x)))
#define D_HasBitSet(x, bit) ((x&bit)==bit)

#define DSizeOf(x) (unsigned int)sizeof(x)
#define DSizeOfBits(type) ((DSizeT)CHAR_BIT * sizeof(type))

#define D_MAKE_UINT16(a, b) ((DUInt16)(((DByte)(((DUInt32)(a)) & 0xff)) | ((DUInt16)((DByte)(((DUInt32)(b)) & 0xff))) << 8))
#define D_MAKE_UINT32(a, b) ((DUInt32)(((DUInt16)(((DUInt32)(a)) & 0xffff)) | ((DUInt32)((DUInt16)(((DUInt32)(b)) & 0xffff))) << 16))
#define D_MAKE_UINT64(a, b) (((static_cast<DUInt64>(a) << 32) + 0x##b##u))
#define D_LOWORD(l) ((DUInt16)(((DUInt32)(l)) & 0xffff))
#define D_HIWORD(l) ((DUInt16)((((DUInt32)(l)) >> 16) & 0xffff))
#define D_LOBYTE(w) ((DByte)(((DUInt32)(w)) & 0xff))
#define D_HIBYTE(w) ((DByte)((((DUInt32)(w)) >> 8) & 0xff))

#define D_AVG(a, b) (((a) + (b) + 1) >> 1)
#define D_ROUND(v) (v>0)?(DInt32)(v+0.5):(DInt32)(v-0.5)


#ifndef D_DISALLOW_COPY_AND_ASSIGN
#define D_DISALLOW_COPY_AND_ASSIGN(TypeName) \
private: \
    TypeName(const TypeName&) = delete;      \
    TypeName& operator=(const TypeName&) = delete;
#endif

#ifndef D_DISALLOW_ALL_DEFAULT
#define D_DISALLOW_ALL_DEFAULT(TypeName)      \
private: \
    TypeName() = delete;   \
    ~TypeName() = delete;  \
    TypeName(const TypeName&) = delete;  \
    TypeName& operator=(const TypeName&) = delete;
#endif
