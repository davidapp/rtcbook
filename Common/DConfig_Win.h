#pragma once

// Windows 下的子平台架构，目前有 X86 X64 IA64 三种架构
#if defined(_M_IX86)
#define BUILD_FOR_X86  1
#elif defined(_M_X64) || defined(_M_AMD64_)
#define BUILD_FOR_X64  1
#elif defined(_M_IA64)
#define BUILD_FOR_IA64 1
#endif

// 是否是 64位
#if defined(_WIN64)
#define TARGET_64BIT 1
#endif

// 调用约定，只适用于X86
#define DX86_STDCALL __stdcall
#define DX86_CCALL __cdecl
#define DX86_FASTCALL __fastcall
#define DX86_THISCALL __thiscall
#define DX86_NAKED __declspec(naked)

// declspec
#define D_NO_VTABLE __declspec(novtable)
#define D_LINK_ONCE __declspec(selectany)
#define D_NO_THROW __declspec(nothrow)
#define D_ALIGN(x) __declspec(align(x))
#define D_NO_RETURN __declspec(noreturn)

// align
#define D_ALIGN_BEST D_ALIGN(SYSTEM_CACHE_ALIGNMENT_SIZE)
#define DAlignOf(x) __alignof(x)

// Always compile using Unicode and Unicode C-Runtime functions
#ifndef UNICODE
    #define UNICODE
#endif

#ifdef UNICODE
    #ifndef _UNICODE
        #define _UNICODE
    #endif
#endif

// 编译警告消除
#define D_WARNING_SUPPRESS(x) __pragma(warning(push)) __pragma(warning(disable: x))
#define D_WARNING_UNSUPPRESS() __pragma(warning(pop))


#ifndef STRICT
#define STRICT 1
#endif

// Windows 版本
// https://docs.microsoft.com/en-us/windows/win32/winprog/using-the-windows-headers?redirectedfrom=MSDN
#include <SDKDDKVer.h>

// Windows Related Headers
#include <winsock2.h>
#include <windows.h>

// Line
#define D_LINE '\r''\n'
#define D_LINES "\r\n"

// File
typedef HANDLE DFileHandle;
#define D_INVALID_FILE INVALID_HANDLE_VALUE

// Path
#define D_PATH_SLASH '\\'
#define D_PATH_SLASHS "\\"

// Socket
typedef SOCKET DSocket;
