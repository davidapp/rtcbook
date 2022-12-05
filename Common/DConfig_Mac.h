//
//  DConfig_Mac.h
//
//  Created by Dai Wei(bmw.dai@gmail.com) on 09/27/2017 for Dream.
//  Copyright 2017. All rights reserved.
//

#pragma once

//Platform
//#define BUILD_FOR_ARM    0
//#define BUILD_FOR_ARM64  0
//#define BUILD_FOR_MIPS   0
//#define BUILD_FOR_MIPS64 0
//#define BUILD_FOR_X86    0
//#define BUILD_FOR_X64       1

//Call Conventions
#define DX86_CCALL
#define DX86_STDCALL
#define DCALL_NAKED

//Export
#define DAPI

#define TARGET_64BIT 1

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

//Mac Versions
#include <AvailabilityMacros.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>    //for read and write
#include <poll.h>    //for poll and ppoll
//#include <sys/epoll.h>    //for epoll API
#include <sys/mman.h>    //for mmap API
#include <sys/select.h> //for fd_set
#include <sys/syscall.h>

// stdarg.h
#include <stdarg.h>
typedef __builtin_va_list DVAList;
#define D_VA_Start(ap, param) __builtin_va_start(ap, param)
#define D_VA_End(ap)          __builtin_va_end(ap)
#define D_VA_Arg(ap, type)    __builtin_va_arg(ap, type)

// Line
#define D_LINE '\n'
#define D_LINES "\n"

// File
typedef int DFileHandle;
#define D_INVALID_FILE -1

// Path
#define D_PATH_SLASH '\\'
#define DW_PATH_SLASH L'\\'
#define D_PATH_SLASHS "\\"
#define DW_PATH_SLASHS L"\\"

// Socket
typedef int DSocket;
#define DBadSocket -1
#define DSockError -1
