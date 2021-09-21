#pragma once

//Platform
//#define BUILD_FOR_ARMV7  0
//#define BUILD_FOR_ARMV7S 0
//#define BUILD_FOR_ARM64  0
//#define BUILD_FOR_X64	 1

#if defined(__arm__) && defined(__ARM_ARCH) && (__ARM_ARCH==7) && defined(__ARM_ARCH_7A) && (__ARM_ARCH_7A==1)
#define BUILD_FOR_ARMV7  1
#define BUILD_FOR_ARMV7S 0
#define BUILD_FOR_ARM64  0
#define BUILD_FOR_X64	 0
#elif defined(__arm__) && defined(__ARM_ARCH) && (__ARM_ARCH==7) && defined(__ARM_ARCH_7S) && (__ARM_ARCH_7S==1)
#define BUILD_FOR_ARMV7  0
#define BUILD_FOR_ARMV7S 1
#define BUILD_FOR_ARM64  0
#define BUILD_FOR_X64	 0
#elif defined(__arm64__) && defined(__ARM64_ARCH_8__) && (__ARM64_ARCH_8__==1)
#define BUILD_FOR_ARMV7  0
#define BUILD_FOR_ARMV7S 0
#define BUILD_FOR_ARM64  1
#define BUILD_FOR_X64	 0
#elif defined(__amd64__)
#define BUILD_FOR_ARMV7  0
#define BUILD_FOR_ARMV7S 0
#define BUILD_FOR_ARM64  0
#define BUILD_FOR_X64	 1
#endif

//64 bit
#if (defined(BUILD_FOR_ARM64) && (BUILD_FOR_ARM64==1)) || (defined(BUILD_FOR_X64) && (BUILD_FOR_X64==1)) 
#define TARGET_64BIT 1
#endif

//Call Conventions
#define DX86_CCALL 
#define DX86_STDCALL 
#define DCALL_NAKED 

//Export
#define DAPI

//Attribute
#ifndef D_NO_RETURN
#define D_NO_RETURN __attribute__((noreturn))
#endif

#ifdef __has_attribute
#   define D_HAS_ATTRIBUTE(x) __has_attribute(x)
#else
#   define D_HAS_ATTRIBUTE(x) 0
#endif

#ifndef D_UNUSED_ATTR
#if D_HAS_ATTRIBUTE(unused)
#define D_UNUSED_ATTR __attribute__((unused))
#else
#define D_UNUSED_ATTR
#endif
#endif

//iOS Versions
#include <Availability.h>

//Header
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>    //for read and write
#include <poll.h>    //for poll and ppoll
//#include <sys/epoll.h>    //for epoll API
#include <sys/mman.h>    //for mmap API
#include <sys/select.h> //for fd_set
#include <sys/syscall.h>


//stdarg
#include <stdarg.h>
typedef va_list DVAList;
#define D_VA_Start va_start
#define D_VA_Arg va_arg
#define D_VA_End va_end

//sizeof
#define DSizeOf(x) (unsigned int)sizeof(x)

//Line
#define D_LINE '\n'
#define D_LINES "\n"

//File
typedef int DFileHandle;
#define D_INVALID_FILE -1

//Path
#define D_PATH_SLASH '\\'
#define DW_PATH_SLASH L'\\'
#define D_PATH_SLASHS "\\"
#define DW_PATH_SLASHS L"\\"

//Socket
typedef int DSocket;

