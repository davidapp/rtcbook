#include "DBmpFile.h"
#include "Base/DFile.h"
#include "Base/DXP.h"

// DBITMAPFILEHEADER(14) + DBITMAPINFOHEADER(40) + 可选PALETTE + RGB_Data
// RGB_Data 部分要注意：1）每一行会按4字节对齐 2）最开始的一行是左下角的
// Multi-byte integers in the Windows BMP format are stored with the least significant bytes first. 
// Data stored in the BMP format consists entirely of complete bytes so bit string ordering is not an issue.

DBool DBmpFile::Load(DCStr strPath, DBmpInfo* info)
{
    DBuffer buf = DFile::FilePath2Buffer(strPath);
    if (buf.IsNull()) return false;

    DReadBuffer bufRead(buf);
    DBuffer fileHead = bufRead.ReadFixBuffer(14); // Sizeof(DBITMAPFILEHEADER)
    DBuffer infoHead = bufRead.ReadFixBuffer(40); // Sizeof(DBITMAPINFOHEADER)
    DBITMAPFILEHEADER* pFileHeader = (DBITMAPFILEHEADER*)fileHead.GetBuf();
    DBITMAPINFOHEADER* pInfoHeader = (DBITMAPINFOHEADER*)infoHead.GetBuf();

    info->imageWidth = pInfoHeader->biWidth;
    info->imageHeight = pInfoHeader->biHeight;
    info->imageBits = pInfoHeader->biBitCount;
    info->imageLineBytes = GetAlignWidth24(pInfoHeader->biWidth);
    DBuffer bufData = bufRead.ReadFixBuffer(buf.GetSize() - 54);
    info->imageBuf = bufData.GetBuf();
    info->fileHead = fileHead.GetBuf();
    info->infoHead = infoHead.GetBuf();

    bufData.Detach();
    fileHead.Detach();
    infoHead.Detach();

    return true;
}

DVoid DBmpFile::Free(DBmpInfo* info)
{
    DBuffer bufFile, bufInfo, bufBitmap;
    bufFile.Attach(info->fileHead);
    bufInfo.Attach(info->infoHead);
    bufBitmap.Attach(info->imageBuf);
    info->fileHead = nullptr;
    info->infoHead = nullptr;
    info->imageBuf = nullptr;
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

std::string DBmpFile::biCompToStr(DUInt32 biCompression)
{
    // https://docs.microsoft.com/en-us/windows/win32/directshow/fourcc-codes
    //if (biCompression == BI_RGB) return "BI_RGB";
    //else if (biCompression == BI_BITFIELDS) return "BI_RGB";
    //TODO
    
    char buf[10] = {};
    char* p = (char*)&biCompression;
#if defined(BUILD_FOR_WINDOWS)
    sprintf_s(buf, 10, "%c%c%c%c", p[0], p[1], p[2], p[3]);
#else
    sprintf(buf, "%c%c%c%c", p[0], p[1], p[2], p[3]);
#endif
    std::string ret = buf;
    return ret;
}

std::string DBmpFile::DumpBitmapFileHeader(void* pFileHeader)
{
    //https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapfileheader
    std::string ret, temp;
    DBITMAPFILEHEADER* p = (DBITMAPFILEHEADER*)pFileHeader;
    if (p == nullptr) return "null";

    temp = "DBITMAPFILEHEADER(14 bytes):\r\n";
    ret += temp;

    temp = "bfType(2 bytes): ";
    ret += temp;
    ret += DXP::UInt16ToStr16(p->bfType, false);
    ret += "\r\n";

    temp = "bfSize(4 bytes): ";
    ret += temp;
    ret += DXP::UInt32ToStr(p->bfType);
    ret += "\r\n";

    temp = "bfReserved1(2 bytes): ";
    ret += temp;
    ret += DXP::UInt16ToStr16(p->bfReserved1);
    ret += "\r\n";

    temp = "bfReserved2(2 bytes): ";
    ret += temp;
    ret += DXP::UInt16ToStr16(p->bfReserved2);
    ret += "\r\n";

    temp = "bfOffBits(4 bytes): ";
    ret += temp;
    ret += DXP::UInt32ToStr(p->bfOffBits);
    ret += "\r\n";

    return ret;
}

std::string DBmpFile::DumpBitmapInfoHeader(void* pFileHeader)
{
    // https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader
    std::string ret, temp;
    DBITMAPINFOHEADER* p = (DBITMAPINFOHEADER*)pFileHeader;
    if (p == nullptr) return "null";

    temp = "DBITMAPINFOHEADER(40 bytes):\r\n";
    ret += temp;

    temp = "biSize(4 bytes): ";
    ret += temp;
    ret += DXP::UInt32ToStr(p->biSize);
    ret += "\r\n";

    temp = "biWidth(4 bytes): ";
    ret += temp;
    ret += DXP::UInt32ToStr(p->biWidth);
    ret += "\r\n";

    temp = "biHeight(4 bytes): ";
    ret += temp;
    ret += DXP::UInt32ToStr(p->biHeight);
    ret += "\r\n";

    temp = "biPlanes(2 bytes): ";
    ret += temp;
    ret += DXP::UInt16ToStr(p->biPlanes);
    ret += "\r\n";

    temp = "biBitCount(2 bytes): ";
    ret += temp;
    ret += DXP::UInt16ToStr(p->biBitCount);
    ret += "\r\n";

    temp = "biCompression(4 bytes): ";
    ret += temp;
    ret += biCompToStr(p->biCompression);
    ret += "\r\n";

    temp = "biSizeImage(4 bytes): ";
    ret += temp;
    ret += DXP::UInt32ToStr(p->biSizeImage);
    ret += "\r\n";

    temp = "biXPelsPerMeter(4 bytes): ";
    ret += temp;
    ret += DXP::UInt32ToStr(p->biXPelsPerMeter);
    ret += "\r\n";

    temp = "biYPelsPerMeter(4 bytes): ";
    ret += temp;
    ret += DXP::UInt32ToStr(p->biYPelsPerMeter);
    ret += "\r\n";

    temp = "biClrUsed(4 bytes): ";
    ret += temp;
    ret += DXP::UInt32ToStr(p->biClrUsed);
    ret += "\r\n";

    temp = "biClrImportant(4 bytes): ";
    ret += temp;
    ret += DXP::UInt32ToStr(p->biClrImportant);
    ret += "\r\n";

    return ret;
}
