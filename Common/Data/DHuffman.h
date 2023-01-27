//
//  DHuffman.h
//
//  Created by Dai Wei(bmw.dai@gmail.com) on 05/23/2021 for Dream.
//  Copyright 2021. All rights reserved.
//

#pragma once

#include "DConfig.h"
#include "DTypes.h"
#include "Base/DString.h"
#include "Data/DData.h"
#include "Base/DBitStream.h"


typedef struct tagCodeLenItem {
    DUInt8 charVal;
    DUInt8 codeLen;
    union {
        DUInt16 freqTableIndex;
        DUInt16 huffCodec;
    };
}CodeLenItem;


class DAPI DHuffTable {
public:
    DHuffTable(DUInt32 id, DBuffer lenCount, DBuffer charArr);
    DBool ReadNextValue(DBitStream& bs, DUInt8& val);
    DVoid PrintCodeItem();
    DVoid PrintDecodeTable();
    static DInt32 Extend(DInt32 enVal, DInt32 readVal);

private:
    DUInt32 m_id;
    CodeLenItem m_codeItem[256];
    DUInt32 m_codeItemSize;
    DUInt16 m_MinCode[17];
    DUInt16 m_MaxCode[17];
    DUInt8  m_FirstValueIndex[17];
};


class DAPI DHuffman
{
public:
    static DData* CountChar(const DStringA& str); // 256 Items -> Count
    static DData* GenCodeLen(DData* pFrequencyTable); // 256 Item -> 1-32
    static DData* GenLengthCount(DData* pCodeLenTable); // 32 Items -> Count
    static DData* Limit16CodeLen(DData* pLenCountTable); // 16 Items -> Count
    static DData* GenCodec(DData* pCodeLenTable); // 256 Item -> Codec

public:
    static DData* ExpandCodeLen(DData* pLengthCountTable, DBuffer charArr);
    static DBuffer Decode(DBuffer clArr, DBuffer charArr, DBuffer cBuffer);

public:
    static DVoid Find2Min(DInt32* arr, DUInt32 arrLen, DUInt32& min1, DUInt32& min2);
};

