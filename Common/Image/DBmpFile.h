//
//  DBmpFile.h
//
//  Created by Dai Wei(bmw.dai@gmail.com) on 05/17/2021 for Dream.
//  Copyright 2021. All rights reserved.
//

#pragma once

#include "DConfig.h"
#include "DTypes.h"
#include "Base/DString.h"
#include "Base/DBuffer.h"
#include "Data/DData.h"

typedef enum
{
	DIB_1BPP,           //   2 color image, palette-based
	DIB_2BPP,           //   4 color image, palttte-based
	DIB_4BPP,           //  16 color image, palette-based
	DIB_4BPPRLE,        //  16 color image, palette-based, RLE compressed
	DIB_8BPP,           // 256 color image, palette-based
	DIB_8BPPRLE,        // 256 color image, palette-based, RLE compressed

	DIB_16RGB555,       // 15 bit RGB color image, 5-5-5
	DIB_16RGB565,       // 16 bit RGB color image, 5-6-5, 1 bit unused
	DIB_24RGB888,       // 24 bit RGB color image, 8-8-8
	DIB_32RGB888,       // 32 bit RGB color image, 8-8-8, 8 bit unused

	DIB_32RGBA8888,     // 32 bit RGBA color image, 8-8-8-8

	DIB_16RGBbitfields, // 16 bit RGB color image, nonstandard bit masks, NT-only
	DIB_32RGBbitfields, // 32 bit RGB color image, nonstandard bit masks, NT-only

	DIB_JPEG,           // embedded JPEG image
	DIB_PNG             // embedded PNG image
} DDIBFormat;


enum DBMPInfo_Data{
	DBMP_UINT32_PIXEL_FORMAT = 0,
	DBMP_UINT32_WIDTH = 1,
	DBMP_UINT32_HEIGHT = 2,
	DBMP_UINT32_BITS = 3,
	DBMP_UINT32_LINE_BYTES = 4,
	DBMP_BUFFER_INFO_BUFFER = 5,
	DBMP_BUFFER_DATA_BUFFER = 6
};


typedef struct tagDBmpInfo {
	// 基本位图信息，位深，宽像素，高像素，内存Buf
	DUInt32 imageBits;
	DUInt32 imageWidth;
	DUInt32 imageHeight;
	DByte* imageBuf;
} DBmpInfo;


class DAPI DBmpFile
{
public:
	static DData* Load(DCStr filePath, DBmpInfo* info);
	static DUInt32 GetRGB(DData* pData, DUInt32 x, DUInt32 y);

public:
    static DUInt32 GetAlignWidth24(DUInt32 w);
	static DBuffer Make24BitBitmap(DUInt32 w, DUInt32 h, DBuffer data);
	static DBuffer Make32BitBitmap(DUInt32 w, DUInt32 h, DBuffer data);


#if defined(D_INCLUDE_TEST) && (D_INCLUDE_TEST==1)
public:
	static DVoid SaveTestBmp24(DCStr filePath);
	static DVoid SaveTestBmp24_Align(DCStr filePath);
	static DVoid SaveTestBmp32(DCStr filePath);
	static DVoid Test();
#endif
};

