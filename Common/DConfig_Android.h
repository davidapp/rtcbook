//
//  DConfig_Android.h
//
//  Created by Dai Wei(bmw.dai@gmail.com) on 06/12/2017 for Dream.
//  Copyright 2017. All rights reserved.
//

#pragma once

//Platform
//#define BUILD_FOR_ARM    0
//#define BUILD_FOR_ARM64  0
//#define BUILD_FOR_MIPS   0
//#define BUILD_FOR_MIPS64 0
//#define BUILD_FOR_X86    0
//#define BUILD_FOR_X64	   1

#if defined(__aarch64__)
#define TARGET_64BIT 1
#endif

//Call Conventions
#define DX86_STDCALL
#define DX86_CCALL 
#define DX86_NAKED

//Export
#define DAPI

//Attribute
#ifndef D_NO_RETURN
    #define D_NO_RETURN __attribute__((noreturn))
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


//malloc free atoi
#include <stdlib.h>

//Android Versions
#include <android/ndk-version.h>

//Thread
#include <pthread.h>	//for pthread_t

//Print
#include <android/log.h>
#define DPrintf(...)  __android_log_print(ANDROID_LOG_INFO, "JNI", __VA_ARGS__)

//Assert
#include <assert.h>
#define DAssert(...) assert(__VA_ARGS__)

//stdarg
#include <stdarg.h>
typedef va_list DVAList;
#define D_VA_Start va_start
#define D_VA_Arg va_arg
#define D_VA_End va_end

//Line
#define D_LINE '\n'
#define D_LINES "\n"

//File
typedef int DFileHandle;
#define D_INVALID_FILE -1
#define D_PATH_SLASH '/'
#define D_PATH_SLASHS "/"

//Socket
typedef int DSocket;

