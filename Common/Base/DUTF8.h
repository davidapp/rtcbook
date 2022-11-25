#pragma once

#include "DTypes.h"
#include <string>

class DUTF8
{
public:
    static DUInt32 UTF8Length16(DUInt16* pUnicode, DUInt32 uBufSize);
    static DUInt32 UTF8Length32(DUInt32* pUnicode61, DUInt32 uBufSize);	//Unicode 6.1

public:
    static std::string  UCS2ToUTF8(DUInt16* pUnicode, DUInt32 uBufSize);
    static std::string  UCS4ToUTF8(DUInt32* pUnicode61, DUInt32 uBufSize);

    static DUInt32 UTF8ToUCS2(DByte* pUTF8Str, DUInt32 uBufSize, DWChar* pDest);
    static std::wstring UTF8ToUCS2(DByte* pUTF8Str, DUInt32 uBufSize);
    static std::wstring UTF8ToUCS2(const std::string& str);

public:
    static DUInt32 isUTF8Lead16(DByte c);
    static DUInt32 isUTF8Lead32(DByte c);
    static DBool   isUTF8Tail(DByte c);

    static DBool   isUTF8(DByte* pUTF8Str, DUInt32 uSize);
    static DUInt32 GetUTF8CharCount(DByte* pUTF8Str, DUInt32 uBufSize);

    static DByte* UTF8CharNext(DByte* pUTF8Str);
    static DByte* UTF8CharPrev(DByte* pUTF8Str, DByte* pBegin);

public:
    static DUInt32 UTF8Encode16(DUInt16 c, DByte* pStr);
    static DUInt32 UTF8Decode16(DByte* pUTF8Str, DUInt16* c);
    static DUInt32 UTF8Encode32(DUInt32 c, DByte* pStr);
    static DUInt32 UTF8Decode32(DByte* pStr, DUInt32* c);

    D_DISALLOW_ALL_DEFAULT(DUTF8)
};

