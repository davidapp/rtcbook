#include "DUTF8.h"

////////////////////////////////////////////////////////////////////
//
// RFC 3629 
// https://tools.ietf.org/html/rfc3629
// Ken Thompson 1992
// 0000 ~ 007F	0~7		0XXXXXXX 1
// 0080 ~ 07FF	8~11	110XXXXX 10XXXXXX 2
// 0800 ~ FFFF	12~16	1110XXXX 10XXXXXX 10XXXXXX 3		   for Basic
//
// 10000~1FFFFF	17~21	11110XXX 10XXXXXX 10XXXXXX 10XXXXXX 4  for Unicode 6.1
// Unicode6.1 0~10FFFF
//
// For UCS-4 
// 200000~3FFFFFF 22~26 111110XX 10XXXXXX 10XXXXXX 10XXXXXX 10XXXXXX 5
// 4000000~7FFFFFFF 27~31 1111110X 10XXXXXX 10XXXXXX 10XXXXXX 10XXXXXX 10XXXXXX 6
//

DUInt32 DUTF8::UTF8Length16(DUInt16* pUnicode, DUInt32 uBufSize)
{
    DUInt32 nCount = 0;
    DUInt32 nSize = 0;
    DUInt16* pStr = pUnicode;
    DUInt16 c = *pStr;
    while (c)
    {
        if (nSize >= uBufSize) break;
        if (c > 0 && c <= 0x7F) nCount++;
        else if (c >= 0x80 && c <= 0x7FF) nCount += 2;
        else if (c >= 0x800) nCount += 3;
        nSize += 2;
        ++pStr;
        c = *pStr;
    }
    return nCount;
}

DUInt32 DUTF8::UTF8Length32(DUInt32* pUnicode6, DUInt32 uBufSize)
{
    DUInt32 nCount = 0;
    DUInt32 nSize = 0;
    DUInt32* pStr = pUnicode6;
    DUInt32 c = *pStr;
    while (c)
    {
        if (nSize >= uBufSize) break;
        if (c > 0 && c <= 0x7F) nCount++;
        else if (c >= 0x80 && c <= 0x7FF) nCount += 2;
        else if (c >= 0x800 && c <= 0xFFFF) nCount += 3;
        else if (c >= 0x10000 && c <= 0x1FFFFF) nCount += 4;
        else if (c >= 0x200000 && c <= 0x3FFFFFF) nCount += 5;
        else if (c >= 0x4000000 && c <= 0x7FFFFFFF) nCount += 6;
        ++pStr;
        c = *pStr;
    }
    return nCount;
}

std::string DUTF8::UCS2ToUTF8(DUInt16* pUnicode, DUInt32 uBufSize)
{
    std::string strRet;
    DUInt32 nCount = UTF8Length16(pUnicode, uBufSize);
    if (nCount > 0)
    {
        strRet.resize(nCount);
        DByte* pStrDest = (DByte*)strRet.c_str();
        DUInt16* pStrSrc = pUnicode;
        DUInt16 c = *pStrSrc;
        while (c)
        {
            DUInt32 nLen = UTF8Encode16(c, pStrDest);
            pStrDest += nLen;
            pStrSrc++;
            c = *pStrSrc;
        }
    }
    return strRet;
}

std::string DUTF8::UCS2ToUTF8(const std::wstring& wstr)
{
    return DUTF8::UCS2ToUTF8((DUInt16*)wstr.c_str(), wstr.length() * 2);
}


std::wstring DUTF8::UTF8ToUCS2(DByte* pUTF8Str, DUInt32 uBufSize)
{
    std::wstring strRet;
    DUInt32 nLen = DUTF8::GetUTF8CharCount(pUTF8Str, uBufSize);
    if (nLen > 0)
    {
        strRet.resize(nLen);
        DUInt32 nIndex = 0;
        DByte* pStart = pUTF8Str;
        while (*pStart)
        {
            DUInt16 c = 0;
            DUInt32 nCharLen = UTF8Decode16(pStart, &c);
            strRet[nIndex] = c;
            nIndex++;
            pStart += nCharLen;
        }
    }
    return strRet;
}

std::wstring DUTF8::UTF8ToUCS2(const std::string& str)
{
    return DUTF8::UTF8ToUCS2((DByte*)str.c_str(), str.length());
}

std::string DUTF8::UCS4ToUTF8(DUInt32* pUnicode61, DUInt32 uBufSize)
{
    std::string strRet;
    DUInt32 nCount = UTF8Length32(pUnicode61, uBufSize);
    if (nCount > 0)
    {
        strRet.resize(nCount);
        DByte* pStrDest = (DByte*)strRet.c_str();
        DUInt32* pStrSrc = pUnicode61;
        DUInt16 c = *pStrSrc;
        while (c)
        {
            DUInt32 nLen = UTF8Encode32(c, pStrDest);
            pStrDest += nLen;
            pStrSrc++;
            c = *pStrSrc;
        }
    }
    return strRet;
}

DUInt32 DUTF8::UTF8Encode16(DUInt16 c, DByte* pStr)
{
    if (c <= 0x7F)
    {
        *pStr = (DByte)c;
        return 1;
    }
    else if (c >= 0x80 && c <= 0x7FF)
    {
        *pStr = ((DByte)(c >> 6)) | 0xC0;
        *(pStr + 1) = ((DByte)(c & 0x3F)) | 0x80;
        return 2;
    }
    else if (c >= 0xD800 && c <= 0xDFFF)	//UTF-16 Surrogate Codec
    {
        //对于替代区中的字符，Windows转换函数的UTF-8，将其编码成了固定的3个字节 0xEFBFBD(0xFFFD 特殊符号)
        //Dream使用一样的策略，但是这里是可以改进的，正确的方式是识别出UTF32编码后，再转为UTF8
        //1平面对应方式为0x00010000 <-> 0xD800:DC00
        //2平面对应方式为0x00020000 <-> 0xD840:DC00
        *pStr = 0xEF;
        *(pStr + 1) = 0xBF;
        *(pStr + 2) = 0xBD;
        return 3;
    }
    else
    {
        *pStr = ((DByte)(c >> 12)) | 0xE0;
        *(pStr + 1) = ((DByte)((c & 0xFC0) >> 6)) | 0x80;
        *(pStr + 2) = ((DByte)(c & 0x3F)) | 0x80;
        return 3;
    }
}

DUInt32 DUTF8::UTF8Encode32(DUInt32 c, DByte* pStr)
{
    if (c > 0 && c <= 0x7F)
    {
        *pStr = (DByte)c;
        return 1;
    }
    else if (c >= 0x80 && c <= 0x7FF)
    {
        *pStr = ((DByte)(c >> 6)) | 0xC0;
        *(pStr + 1) = ((DByte)(c & 0x3F)) | 0x80;
        return 2;
    }
    else if (c >= 0x800 && c <= 0xFFFF)
    {
        *pStr = ((DByte)(c >> 12)) | 0xE0;
        *(pStr + 1) = ((DByte)((c & 0xFC0) >> 6)) | 0x80;
        *(pStr + 2) = ((DByte)(c & 0x3F)) | 0x80;
        return 3;
    }
    else if (c >= 0x10000 && c <= 0x1FFFFF)
    {
        *pStr = ((DByte)(c >> 18)) | 0xF0;
        *(pStr + 1) = ((DByte)((c & 0x3F000) >> 12)) | 0x80;
        *(pStr + 2) = ((DByte)(c & 0xFC0) >> 6) | 0x80;
        *(pStr + 3) = ((DByte)(c & 0x3F)) | 0x80;
        return 4;
    }
    else if (c >= 0x200000 && c <= 0x3FFFFFF)
    {
        *pStr = ((DByte)(c >> 24)) | 0xF8;
        *(pStr + 1) = ((DByte)((c & 0xFC0000) >> 18)) | 0x80;
        *(pStr + 2) = ((DByte)((c & 0x3F000) >> 12)) | 0x80;
        *(pStr + 3) = ((DByte)(c & 0xFC0) >> 6) | 0x80;
        *(pStr + 4) = ((DByte)(c & 0x3F)) | 0x80;
        return 5;
    }
    else if (c >= 0x4000000 && c <= 0x7FFFFFFF)
    {
        *pStr = ((DByte)(c >> 30)) | 0xFC;
        *(pStr + 1) = ((DByte)((c & 0x3F000000) >> 24)) | 0x80;
        *(pStr + 2) = ((DByte)((c & 0xFC0000) >> 18)) | 0x80;
        *(pStr + 3) = ((DByte)((c & 0x3F000) >> 12)) | 0x80;
        *(pStr + 4) = ((DByte)(c & 0xFC0) >> 6) | 0x80;
        *(pStr + 5) = ((DByte)(c & 0x3F)) | 0x80;
        return 6;
    }
    *pStr = 0;
    return 0;
}

DUInt32 DUTF8::UTF8Decode16(DByte* pUTF8Str, DUInt16* c)
{
    DUInt32 nLen = isUTF8Lead16(pUTF8Str[0]);
    if (nLen == 1)
    {
        *c = pUTF8Str[0] & 0x7F;
    }
    else if (nLen == 2)
    {
        DUInt16 t1 = pUTF8Str[0] & 0x1F;
        *c = (t1 << 6) + (pUTF8Str[1] & 0x3F);
    }
    else if (nLen == 3)
    {
        DUInt16 t1 = pUTF8Str[0] & 0x0F;
        DUInt16 t2 = pUTF8Str[1] & 0x3F;
        *c = (t1 << 12) + (t2 << 6) + (pUTF8Str[2] & 0x3F);
    }
    return nLen;
}

DUInt32 DUTF8::UTF8Decode32(DByte* pUTF8Str, DUInt32* c)
{
    DUInt32 nLen = isUTF8Lead16(pUTF8Str[0]);
    if (nLen == 1)
    {
        *c = pUTF8Str[0] & 0x7F;
    }
    else if (nLen == 2)
    {
        DUInt16 t1 = pUTF8Str[0] & 0x1F;
        *c = (t1 << 6) + (pUTF8Str[1] & 0x3F);
    }
    else if (nLen == 3)
    {
        DUInt16 t1 = pUTF8Str[0] & 0x0F;
        DUInt16 t2 = pUTF8Str[1] & 0x3F;
        *c = (t1 << 12) + (t2 << 6) + (pUTF8Str[2] & 0x3F);
    }
    else if (nLen == 4)
    {
        DUInt32 t1 = pUTF8Str[0] & 0x07;
        DUInt32 t2 = pUTF8Str[1] & 0x3F;
        DUInt32 t3 = pUTF8Str[2] & 0x3F;
        *c = (t1 << 18) + (t2 << 12) + (t3 << 6) + (pUTF8Str[3] & 0x3F);
    }
    else if (nLen == 5)
    {
        DUInt32 t1 = pUTF8Str[0] & 0x03;
        DUInt32 t2 = pUTF8Str[1] & 0x3F;
        DUInt32 t3 = pUTF8Str[2] & 0x3F;
        DUInt32 t4 = pUTF8Str[3] & 0x3F;
        *c = (t1 << 24) + (t2 << 18) + (t3 << 12) + (t4 << 6) + (pUTF8Str[4] & 0x3F);
    }
    else if (nLen == 6)
    {
        DUInt32 t1 = pUTF8Str[0] & 0x01;
        DUInt32 t2 = pUTF8Str[1] & 0x3F;
        DUInt32 t3 = pUTF8Str[2] & 0x0F;
        DUInt32 t4 = pUTF8Str[3] & 0x3F;
        DUInt32 t5 = pUTF8Str[4] & 0x3F;
        *c = (t1 << 30) + (t2 << 24) + (t3 << 18) + (t4 << 12) + (t5 << 6) + (pUTF8Str[5] & 0x3F);
    }
    return nLen;
}

DUInt32 DUTF8::isUTF8Lead16(DByte c)
{
    if ((c & 0x80) == 0)
    {
        return 1;
    }
    else if ((c & 0xE0) == 0xC0)
    {
        return 2;
    }
    else if ((c & 0xF0) == 0xE0)
    {
        return 3;
    }
    return 0;
}

DUInt32 DUTF8::isUTF8Lead32(DByte c)
{
    if ((c & 0x80) == 0)
    {
        return 1;
    }
    else if ((c & 0xE0) == 0xC0)
    {
        return 2;
    }
    else if ((c & 0xF0) == 0xE0)
    {
        return 3;
    }
    else if ((c & 0xF8) == 0xF0)
    {
        return 4;
    }
    else if ((c & 0xFC) == 0xF8)
    {
        return 5;
    }
    else if ((c & 0xFE) == 0xFC)
    {
        return 6;
    }
    return 0;
}

DBool DUTF8::isUTF8Tail(DByte c)
{
    if ((c & 0xC0) == 0x80)
    {
        return true;
    }
    return false;
}

DBool DUTF8::isUTF8(DByte* pUTF8Str, DUInt32 uSize)
{
    DBool bRes = true;
    DByte* pStart = pUTF8Str;
    DUInt32 uCount = 0;
    while (*pStart)
    {
        DUInt32 nLen = isUTF8Lead32(*pStart);
        if (nLen == 0)
        {
            bRes = false;
        }
        else
        {
            for (int i = 1; i < (int)nLen; i++)
            {
                if (uCount + i > uSize)
                {
                    bRes = false;
                    break;
                }

                if (!isUTF8Tail(*(pStart + i)))
                {
                    bRes = false;
                    break;
                }
            }
        }
        if (!bRes) break;
        uCount += nLen;
        if (uCount >= uSize) break;
        pStart = pStart + nLen;
    }
    return bRes;
}

DUInt32 DUTF8::GetUTF8CharCount(DByte* pUTF8Str, DUInt32 uBufSize)
{
    DBool bRes = true;
    DUInt32 nCount = 0;
    DByte* pStart = pUTF8Str;
    while (*pStart)
    {
        DUInt32 nLen = isUTF8Lead16(*pStart);
        if (nLen == 0)
        {
            bRes = false;
        }
        else
        {
            for (int i = 1; i < (int)nLen; i++)
            {
                if (!isUTF8Tail(*(pStart + i)))
                {
                    bRes = false;
                    break;
                }
            }
        }
        if (!bRes) break;
        pStart = pStart + nLen;
        nCount++;
    }
    if (!bRes) return 0;
    return nCount;
}

//End with \0
DByte* DUTF8::UTF8CharNext(DByte* pUTF8Str)
{
    DByte* pStr = pUTF8Str;
    DByte c = *pStr;
    DUInt32 nLen = DUTF8::isUTF8Lead32(c);
    if (nLen > 0)
    {
        return pStr + nLen;
    }
    else
    {
        do {
            if (c == 0) break;
            pStr++;
            c = *pStr;
        } while ((nLen = DUTF8::isUTF8Lead32(c)) != 0);
        return pStr;
    }
}

DByte* DUTF8::UTF8CharPrev(DByte* pUTF8Str, DByte* pBegin)
{
    if (pUTF8Str <= pBegin) return pUTF8Str;
    DByte* pStr = pUTF8Str - 1;
    DByte c = *pStr;
    DUInt32 nLen = DUTF8::isUTF8Lead32(c);
    while (nLen == 0 && pStr >= pBegin)
    {
        pStr--;
        c = *pStr;
        nLen = DUTF8::isUTF8Lead32(c);
    }
    return pStr;
}

