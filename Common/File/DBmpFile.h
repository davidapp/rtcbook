#pragma once

#include "DTypes.h"
#include "Base/DBuffer.h"

typedef DUInt32 DColor;
#define DRGB(r,g,b) ((DUInt32)(((DUInt8)(r)|((DUInt16)((DUInt8)(g))<<8))|(((DUInt32)(DUInt8)(b))<<16)))

#include <pshpack2.h>
typedef struct tagDBITMAPFILEHEADER {
    DUInt16 bfType;			// 0x4D42 'BM'
    DUInt32 bfSize;			// FileSize
    DUInt16 bfReserved1;	// 0
    DUInt16 bfReserved2;	// 0
    DUInt32 bfOffBits;		// Offset of Pixel Data
} DBITMAPFILEHEADER;
// 2+4+2+2+4 = 14

typedef struct tagDBITMAPINFOHEADER {
    DUInt32 biSize;			// Sizeof(DBITMAPINFOHEADER) = 40
    DInt32 biWidth;
    DInt32 biHeight;
    DUInt16 biPlanes;		// Always 1
    DUInt16 biBitCount;		// 1 4 8 16 24 32
    DUInt32 biCompression;	// BI_RGB
    DUInt32 biSizeImage;	// 0
    DInt32 biXPelsPerMeter;	// pixel/meter
    DInt32 biYPelsPerMeter;	// pixel/meter
    DUInt32 biClrUsed;		// 0
    DUInt32 biClrImportant;	// 0
} DBITMAPINFOHEADER;
#include <poppack.h>

typedef struct tagDBmpInfo {
    DUInt32 pixelFormat;
    DUInt32 imageLineBytes;
    // 基本位图信息，位深，宽像素，高像素，内存Buf
    DUInt32 imageBits;
    DUInt32 imageWidth;
    DUInt32 imageHeight;
    DByte* imageBuf;
    DByte* fileHead;
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

public:
    static std::string DumpBitmapFileHeader(void* pFileHeader); // DBITMAPFILEHEADER
    static std::string DumpBitmapInfoHeader(void* pInfoHeader); // DBITMAPINFOHEADER
};
