﻿#include "DBmpFile.h"
#include "Base/DFile.h"

// DBITMAPFILEHEADER(14) + DBITMAPINFOHEADER(40) + 可选PALETTE + RGB_Data
// RGB_Data 部分要注意：1）每一行会按4字节对齐 2）最开始的一行是左下角的
// Multi-byte integers in the Windows BMP format are stored with the least significant bytes first. 
// Data stored in the BMP format consists entirely of complete bytes so bit string ordering is not an issue.

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


DBool DBmpFile::Load(DCStr strPath, DBmpInfo* info)
{
    DBuffer buf = DFile::FilePath2Buffer(strPath);
    if (buf.IsNull()) return false;

    DReadBuffer bufRead(buf);
    DBuffer fileHead = bufRead.ReadFixBuffer(14); // Sizeof(DBITMAPFILEHEADER)
    DBuffer infoHead = bufRead.ReadFixBuffer(40); // Sizeof(DBITMAPINFOHEADER)
    DBITMAPFILEHEADER* pFileHeader = (DBITMAPFILEHEADER*)fileHead.GetBuf();
    DBITMAPINFOHEADER* pInfoHeader = (DBITMAPINFOHEADER*)infoHead.GetBuf();

    info->infoHead = infoHead.GetBuf();
    info->imageWidth = pInfoHeader->biWidth;
    info->imageHeight = pInfoHeader->biHeight;
    info->imageBits = pInfoHeader->biBitCount;
    info->imageLineBytes = GetAlignWidth24(pInfoHeader->biWidth);
    DBuffer bufData = bufRead.ReadFixBuffer(buf.GetSize() - 54);
    info->imageBuf = bufData.GetBuf();

    bufData.Detach();
    infoHead.Detach();

    return true;
}

DVoid DBmpFile::Free(DBmpInfo* info)
{
    DBuffer bufHead, bufBitmap;
    bufHead.Attach(info->infoHead);
    bufBitmap.Attach(info->imageBuf);
}

DUInt32 DBmpFile::GetRGB(DBmpInfo* pInfo, DUInt32 x, DUInt32 y)
{
    DUInt32 bits = pInfo->imageBits;
    DUInt32 line = pInfo->imageLineBytes;
    DUInt32 offset = y * line + x * (bits / 8);
    DByte* pBuf = pInfo->imageBuf + offset;
    return DRGB(pBuf[2], pBuf[1], pBuf[0]);
}

DUInt32 DBmpFile::GetAlignWidth24(DUInt32 w)
{
    DInt32 w4 = (w * 3) % 4;
    if (w4 != 0) {
        w4 = 4 - w4;
    }
    return w * 3 + w4;
}

DBuffer DBmpFile::Make24BitBitmap(DUInt32 w, DUInt32 h, DBuffer data)
{
    // Align w to 4 bytes
    DInt32 w4 = GetAlignWidth24(w);

    DGrowBuffer gb;
    gb.AddUInt8('B');
    gb.AddUInt8('M');
    gb.AddUInt32(14 + 40 + w4 * h);
    gb.AddUInt16(0);
    gb.AddUInt16(0);
    gb.AddUInt32(54);

    gb.AddUInt32(40);
    gb.AddUInt32(w);
    gb.AddUInt32(h);
    gb.AddUInt16(1);
    gb.AddUInt16(24);
    gb.AddUInt32(0);
    gb.AddUInt32(w4 * h);
    gb.AddUInt32(0);
    gb.AddUInt32(0);
    gb.AddUInt32(0);
    gb.AddUInt32(0);

    gb.AddFixBuffer(data);

    DBuffer buf = gb.Finish();
    return buf;
}

DBuffer DBmpFile::Make32BitBitmap(DUInt32 w, DUInt32 h, DBuffer data)
{
    DGrowBuffer gb;
    gb.AddUInt8('B');
    gb.AddUInt8('M');
    gb.AddUInt32(14 + 40 + w * h * 4);
    gb.AddUInt16(0);
    gb.AddUInt16(0);
    gb.AddUInt32(54);

    gb.AddUInt32(40);
    gb.AddUInt32(w);
    gb.AddUInt32(h);
    gb.AddUInt16(1);
    gb.AddUInt16(32);
    gb.AddUInt32(0);    // BI_RGB = 0
    gb.AddUInt32(w * h * 4);
    gb.AddUInt32(0);
    gb.AddUInt32(0);
    gb.AddUInt32(0);
    gb.AddUInt32(0);

    gb.AddFixBuffer(data);
    DBuffer buf = gb.Finish();
    return buf;
}
