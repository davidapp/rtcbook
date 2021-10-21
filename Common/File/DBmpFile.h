#pragma once

#include "DTypes.h"
#include "Base/DBuffer.h"

typedef DUInt32 DColor;
#define DRGB(r,g,b) ((DUInt32)(((DUInt8)(r)|((DUInt16)((DUInt8)(g))<<8))|(((DUInt32)(DUInt8)(b))<<16)))


typedef struct tagDBmpInfo {
    DUInt32 pixelFormat;
    DUInt32 imageLineBytes;
    // 基本位图信息，位深，宽像素，高像素，内存Buf
    DUInt32 imageBits;
    DUInt32 imageWidth;
    DUInt32 imageHeight;
    DByte* imageBuf;
    DByte* infoHead;
} DBmpInfo;

class DBmpFile
{
public:
    static DBool Load(DCStr filePath, DBmpInfo* info);
    static DUInt32 GetRGB(DBmpInfo* pData, DUInt32 x, DUInt32 y);
    static DVoid Free(DBmpInfo* info);

public:
    static DUInt32 GetAlignWidth24(DUInt32 w);
    static DBuffer Make24BitBitmap(DUInt32 w, DUInt32 h, DBuffer data);
    static DBuffer Make32BitBitmap(DUInt32 w, DUInt32 h, DBuffer data);
};
