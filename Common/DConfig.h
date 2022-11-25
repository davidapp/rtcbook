#pragma once

// Platforms
#if defined(_MSC_VER)
#define BUILD_FOR_WINDOWS 1
#elif defined(__APPLE__) && defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__)
#define BUILD_FOR_IOS 1
#elif defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__)
#define BUILD_FOR_MAC 1
#elif defined(__ANDROID__)
#define BUILD_FOR_ANDROID 1
#else
#define BUILD_FOR_LINUX 1
#endif

// Windows
#if defined(BUILD_FOR_WINDOWS)
#include "DConfig_Win.h"
#endif

// iOS
#if defined(BUILD_FOR_IOS)
#include "DConfig_iOS.h"
#endif

// Mac
#if defined(BUILD_FOR_MAC)
#include "DConfig_Mac.h"
#endif
