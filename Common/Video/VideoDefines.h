#pragma once

#include "DTypes.h"

enum class DPixelFmt {
    Unknown,
    RGB24,
    ARGB,
    BGRA,
    RGB565,
    I420,
    IYUV,
    YUY2
};

#if defined(BUILD_FOR_WINDOWS)

#pragma pack(push, 1)

typedef struct tagAMMediaType
{
    GUID majortype;
    GUID subtype;
    BOOL bFixedSizeSamples;
    BOOL bTemporalCompression;
    ULONG lSampleSize;
    GUID formattype;
    IUnknown* pUnk;
    ULONG cbFormat;
    /* [size_is] */ BYTE* pbFormat;
} DAM_MEDIA_TYPE;

#pragma pack(pop)

#endif