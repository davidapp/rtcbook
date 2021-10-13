//
//  DBmpFile.cpp
//
//  Created by Dai Wei(bmw.dai@gmail.com) on 05/17/2021 for Dream.
//  Copyright 2021. All rights reserved.
//

#include "DBmpFile.h"
#include "Base/DBuffer.h"
#include "Base/DFile.h"
#include "Codec/DColorSpace.h"

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


DData* DBmpFile::Load(DCStr strPath, DBmpInfo* info)
{
    DBuffer buf = DFile::FilePath2Buffer(strPath);
    if (buf.IsNull()) return nullptr;

    DReadBuffer bufRead(buf);
    DBuffer fileHead = bufRead.ReadFixBuffer(14); // Sizeof(DBITMAPFILEHEADER)
    DBuffer infoHead = bufRead.ReadFixBuffer(40); // Sizeof(DBITMAPINFOHEADER)
    DBITMAPFILEHEADER* pFileHeader = (DBITMAPFILEHEADER*)fileHead.GetBuf();
    DBITMAPINFOHEADER* pInfoHeader = (DBITMAPINFOHEADER*)infoHead.GetBuf();

    DData* pDataRet = DData::CreateDData(6);
    pDataRet->AddUInt32(DBMP_UINT32_PIXEL_FORMAT, (DUInt32)DPixelFormat::k32RGBA);
    pDataRet->AddUInt32(DBMP_UINT32_WIDTH, pInfoHeader->biWidth);
    pDataRet->AddUInt32(DBMP_UINT32_HEIGHT, pInfoHeader->biHeight);
    pDataRet->AddUInt32(DBMP_UINT32_BITS, pInfoHeader->biBitCount);
    pDataRet->AddUInt32(DBMP_UINT32_LINE_BYTES, GetAlignWidth24(pInfoHeader->biWidth));
    DBuffer bufData = bufRead.ReadFixBuffer(buf.GetSize() - 54);
    pDataRet->AddBuffer(DBMP_BUFFER_INFO_BUFFER, infoHead);
    pDataRet->AddBuffer(DBMP_BUFFER_DATA_BUFFER, bufData);

    return pDataRet;
}

DUInt32 DBmpFile::GetRGB(DData* pData, DUInt32 x, DUInt32 y)
{
    DBuffer bufData = pData->GetBuffer(DBMP_BUFFER_DATA_BUFFER);
    DUInt32 bits = pData->GetUInt32(DBMP_UINT32_BITS);
    DUInt32 line = pData->GetUInt32(DBMP_UINT32_LINE_BYTES);
    DUInt32 offset = y * line + x * (bits / 8);
    DByte* pBuf = bufData.GetBuf() + offset;
    return DColorSpace::MakeRGB(pBuf[2], pBuf[1], pBuf[0]);
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



#if defined(D_INCLUDE_TEST) && (D_INCLUDE_TEST==1)

DVoid DBmpFile::SaveTestBmp24(DCStr filePath)
{
    DBuffer pixelBuffer(256 * 256 * 3);
    pixelBuffer.Zero();
    for (int i = 0; i < 256 * 256 / 3; i++) {
        pixelBuffer.SetAt(i * 3, 0);		//B
        pixelBuffer.SetAt(i * 3 + 1, 0);	//G
        pixelBuffer.SetAt(i * 3 + 2, 255);	//R
    }

    DBuffer filebuf = Make24BitBitmap(256, 256, pixelBuffer);

    DFile file;
    file.OpenFileRW(filePath, DFILE_OPEN_ALWAYS);
    file.Write(filebuf);
    file.Close();
}

DVoid DBmpFile::SaveTestBmp24_Align(DCStr filePath)
{
    DUInt32 line = GetAlignWidth24(30);
    DBuffer pixelBuffer(line * 30);
    pixelBuffer.Zero();
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 30; j++) {
            pixelBuffer.SetAt(i * line + j * 3, 0);		    //B
            pixelBuffer.SetAt(i * line + j * 3 + 1, 0);	    //G
            pixelBuffer.SetAt(i * line + j * 3 + 2, 255);	//R
        }
    }

    DBuffer filebuf = Make24BitBitmap(30, 30, pixelBuffer);

    DFile file;
    file.OpenFileRW(filePath, DFILE_OPEN_ALWAYS);
    file.Write(filebuf);
    file.Close();
}

DVoid DBmpFile::SaveTestBmp32(DCStr filePath)
{
    DBuffer pixelBuffer(256 * 256 * 4);
    pixelBuffer.Zero();
    for (int i = 0; i < 256 * 256 / 3; i++) {
        pixelBuffer.SetAt(i * 4, 0);		//B
        pixelBuffer.SetAt(i * 4 + 1, 0);	//G
        pixelBuffer.SetAt(i * 4 + 2, 255);	//R
        pixelBuffer.SetAt(i * 4 + 3, 0);	//A
    }

    DBuffer filebuf = Make32BitBitmap(256, 256, pixelBuffer);

    DFile file;
    file.OpenFileRW(filePath, DFILE_OPEN_ALWAYS);
    file.Write(filebuf);
    file.Close();
}

DVoid DBmpFile::Test()
{
    DData* pData = DBmpFile::Load("..\\..\\Data\\Image\\24bit.bmp", nullptr);
    if (pData) {
        pData->Dump();
        pData->Release();
    }
}

#endif
