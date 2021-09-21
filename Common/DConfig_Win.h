#pragma once

// Windows 下的子平台架构，目前有 X86 X64 IA64 三种架构
#if defined(_M_IX86)
#define BUILD_FOR_X86  1
#define BUILD_FOR_X64  0
#define BUILD_FOR_IA64 0
#elif defined(_M_X64) || defined(_M_AMD64_)
#define BUILD_FOR_X86  0
#define BUILD_FOR_X64  1
#define BUILD_FOR_IA64 0
#elif defined(_M_IA64)
#define BUILD_FOR_X86  0
#define BUILD_FOR_X64  0
#define BUILD_FOR_IA64 1
#endif

// 是否是 64位
#if defined(_WIN64)
#define TARGET_64BIT 1
#endif

// 内存页面的大小
#if defined(_M_IX64) || defined(_M_IA64)
#define D_PAGESIZE 8*1024	//8k
#else
#define D_PAGESIZE 4*1024	//4k
#endif

// 调用约定，只适用于X86
#define DX86_STDCALL __stdcall		//small when many places call it. R2L_CALLEE_RETN
#define DX86_CCALL __cdecl			//default. R2L_CALLER_ADDESP
#define DX86_FASTCALL __fastcall	//R2L using ecx and edx for v1 and v2
#define DX86_THISCALL __thiscall	//member functions. R2L_USING_ECX
#define DX86_NAKED __declspec(naked)	//No prolog and epilog


// declspec
///////////////////////////////////////////////////////////////////////////////
// __declspec(novtable) is used on a class declaration to prevent the vtable
// pointer from being initialized in the constructor and destructor for the
// class.  This has many benefits because the linker can now eliminate the
// vtable and all the functions pointed to by the vtable.  Also, the actual
// constructor and destructor code are now smaller.
///////////////////////////////////////////////////////////////////////////////
// This should only be used on a class that is not directly createable but is
// rather only used as a base class.  Additionally, the constructor and
// destructor (if provided by the user) should not call anything that may cause
// a virtual function call to occur back on the object.
///////////////////////////////////////////////////////////////////////////////
// By default, the wizards will generate new ATL object classes with this
// attribute (through the ATL_NO_VTABLE macro).  This is normally safe as long
// the restriction mentioned above is followed.  It is always safe to remove
// this macro from your class, so if in doubt, remove it.
///////////////////////////////////////////////////////////////////////////////
#define D_NO_VTABLE __declspec(novtable)
#define D_LINK_ONCE __declspec(selectany)
#define D_NO_THROW __declspec(nothrow)
#define D_ALIGN(x) __declspec(align(x))
#define D_ALIGN_BEST D_ALIGN(SYSTEM_CACHE_ALIGNMENT_SIZE)

#if defined(DLL_EXPORTS) // inside DLL
# define DAPI __declspec(dllexport)
#else // outside DLL
# define DAPI __declspec(dllimport)
#endif // DLIBRARY_EXPORT


#ifndef D_SOURCE_MODE
#define D_SOURCE_MODE	//Default
#endif

#if defined(D_SOURCE_MODE) || defined(_LIB)
#undef DAPI
#define DAPI 
#endif

// Attribute
#ifndef D_NO_RETURN
#define D_NO_RETURN __declspec(noreturn)
#endif

#ifdef __has_attribute
#define D_HAS_ATTRIBUTE(x) __has_attribute(x)
#else
#define D_HAS_ATTRIBUTE(x) 0
#endif

#ifndef D_UNUSED_ATTR
#if D_HAS_ATTRIBUTE(unused)
    #define D_UNUSED_ATTR __attribute__((unused))
#else
    #define D_UNUSED_ATTR
#endif
#endif


// Always compile using Unicode
#ifndef UNICODE
	#define UNICODE
#endif

// When using Unicode Windows functions, use Unicode C-Runtime functions too.
#ifdef UNICODE
	#ifndef _UNICODE
		#define _UNICODE
	#endif
#endif

// 编译警告消除
#define DWARNING_SUPPRESS(x) __pragma(warning(push)) __pragma(warning(disable: x))
#define DWARNING_UNSUPPRESS() __pragma(warning(pop))

// Warning from shlobj.h on VS2015(Windows XP)
#pragma warning (disable: 4091)

// Used inside COM methods that do not want to throw
#ifndef DENSURE_RETURN_VAL
#define DENSURE_RETURN_VAL(expr, val)    \
	do {                                 \
	int condVal=!!(expr);                \
	DAssert(condVal);                    \
	if(!(condVal)) return val;           \
	} while (0) 
#endif // DENSURE_RETURN_VAL

// 坐标获取方法
#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lParam)	((int)(short)LOWORD(lParam))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lParam)	((int)(short)HIWORD(lParam))
#endif

// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.
#ifndef STRICT
#define STRICT 1
#endif
//#define WIN32_LEAN_AND_MEAN	//BSTR

// Windows 版本
// https://docs.microsoft.com/en-us/windows/win32/winprog/using-the-windows-headers?redirectedfrom=MSDN
#include <SDKDDKVer.h>

//Windows Related
#include <winsock2.h>
#include <windows.h>
#include <Tlhelp32.h>		//for Toolhelp functions
#include <Sddl.h>			//for ConvertStringSecurityDescriptorToSecurityDescriptorW
#include <Psapi.h>			//for GetProcessMemoryInfo
#include <shlobj.h>			//for Shell API


//stdarg
#include <vadefs.h>
typedef char* DVAList;
#define D_VA_Start __crt_va_start
#define D_VA_Arg __crt_va_arg
#define D_VA_End __crt_va_end

//alignof
#define DAlignOf(x) __alignof(x)

//Line
#define D_LINE '\r''\n'
#define D_LINES "\r\n"

//File
typedef HANDLE DFileHandle;
#define D_INVALID_FILE INVALID_HANDLE_VALUE

//Path
#define D_PATH_SLASH '\\'
#define DW_PATH_SLASH L'\\'
#define D_PATH_SLASHS "\\"
#define DW_PATH_SLASHS L"\\"

//Socket
typedef SOCKET DSocket;
