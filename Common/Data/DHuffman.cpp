//
//  DHuffman.cpp
//
//  Created by Dai Wei(bmw.dai@gmail.com) on 05/23/2021 for Dream.
//  Copyright 2021. All rights reserved.
//

#include "DHuffman.h"
#include "Base/DMemAlloc.h"
#include "Base/DBitStream.h"
#include "Base/DStdLib.h"

/////////////////////////////////////////////////////////////////////////
// DHuffTable

DHuffTable::DHuffTable(DUInt32 id, DBuffer lenCount, DBuffer charArr)
{
    m_id = id;
    for (DUInt32 i = 0; i < 256; i++) {
        m_codeItem[i].charVal = 0;
        m_codeItem[i].codeLen = 0;
        m_codeItem[i].huffCodec = 0;
    }
    for (DUInt32 i = 0; i < 17; i++) {
        m_MinCode[i] = 0;
        m_MaxCode[i] = 0;
        m_FirstValueIndex[i] = 0;
    }

    // 先得到每个字符对应的长度是多少
    DUInt8 codeLen[256] = {};
    DUInt32 kIndex = 0;
    for (DUInt32 i = 1; i <= 16; i++) {
        DUInt8 count = lenCount.GetAt(i - 1);
        for (DUInt32 j = 0; j < count; j++) {
            DUInt8 ch = charArr.GetAt(kIndex++);
            codeLen[ch] = i;
        }
    }
    m_codeItemSize = kIndex;

    // 按顺序放入 codeItem 中
    for (DUInt32 i = 0; i < m_codeItemSize; i++) {
        m_codeItem[i].charVal = charArr.GetAt(i);
        m_codeItem[i].codeLen = codeLen[m_codeItem[i].charVal];
    }

    // 生成对应的 Huffman 编码
    DUInt32 huffCodeCounter = 0;
    DUInt32 codeLenCounter = 1;
    for (DUInt32 i = 0; i < m_codeItemSize;) {
        if (m_codeItem[i].codeLen == codeLenCounter) {
            m_codeItem[i++].huffCodec = huffCodeCounter;
            huffCodeCounter = huffCodeCounter + 1;
        }
        else {
            huffCodeCounter = huffCodeCounter << 1;
            codeLenCounter = codeLenCounter + 1;
        }
    }

    // 构造对应的解码表
    DUInt32 uIndex = 0;	// 1-16
    for (DUInt32 i = 0; i < m_codeItemSize; i++) {
        if (uIndex != m_codeItem[i].codeLen) {
            uIndex = m_codeItem[i].codeLen;
            m_MinCode[uIndex] = m_codeItem[i].huffCodec;
            m_FirstValueIndex[uIndex] = i + 1;
            m_MaxCode[uIndex] = m_codeItem[i].huffCodec;
        }
        else {
            m_MaxCode[uIndex] = m_codeItem[i].huffCodec;
        }
    }
}

DBool DHuffTable::ReadNextValue(DBitStream& bs, DUInt8& val)
{
    if (bs.IsFinish()) {
        return false;
    }

    // Read a huffman code => Encoded Value(DC: 0-11, AC low 4:1-10)
    DUInt32 code = 0;
    DUInt32 codeLength = 0;
    DInt32 enValue = 255;
    DBool bb;
    while (codeLength <= 16)
    {
        code = code << 1;
        bs.ReadBit(&bb);
        code = code | (bb ? 1 : 0);
        codeLength = codeLength + 1;
        if ((code >= m_MinCode[codeLength]) && (code <= m_MaxCode[codeLength])) {
            if (m_FirstValueIndex[codeLength] > 0) {
                DUInt32 index = m_FirstValueIndex[codeLength] + code - m_MinCode[codeLength];
                enValue = m_codeItem[index - 1].charVal;
                break;
            }
        }
    }
    val = enValue;
    return true;
}

DVoid DHuffTable::PrintCodeItem()
{
    for (DUInt32 i = 0; i < m_codeItemSize; i++) {
        DPrintf("Item:%3d Char:0x%02X Len:%2d Code:%d\r\n", i, m_codeItem[i].charVal, m_codeItem[i].codeLen, m_codeItem[i].huffCodec);
    }
    DPrintf("\r\n");
}

DVoid DHuffTable::PrintDecodeTable()
{
    for (int i = 1; i <= 16; i++) {
        DPrintf("Len:%d Min:%d Max:%d First:%d\r\n", i, m_MinCode[i], m_MaxCode[i], m_FirstValueIndex[i]);
    }
    DPrintf("\r\n");
}

//[Encoded Value] [DC Value Range]
//[0] [0]
//[1] [-1, 1]
//[2] [-3,-2, 2,3]
//[3] [-7..-4, 4..7]
//[4] [-15..-8, 8..15]
//[5] [-31..-16, 16..31]
//[6] [-63..-32, 32..63]
//[7] [-127..-64, 64..127]
//[8] [-255..-128, 128..255]
//[9] [-511..-256, 256..511]
//[10] [-1023..-512, 512..1023]
//[11] [-2047..-1024, 1024..2047]

//[Magnitude Value] [AC Value Range] 
//[1] [-1, 1]
//[2] [-3,-2, 2,3]
//[3] [-7..-4, 4..7]
//[4] [-15..-8, 8..15]
//[5] [-31..-16, 16..31]
//[6] [-63..-32, 32..63]
//[7] [-127..-64, 64..127]
//[8] [-255..-128, 128..255]
//[9] [-511..-256, 256..511]
//[10] [-1023..-512, 512..1023]
//same as DC [1-10]

static const DInt32 extend_test[16] =   /* entry n is 2**(n-1) */
{ 0, 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
  0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000 };

static const DInt32 extend_offset[16] = /* entry n is (-1 << n) + 1 */
{ 0, ((-1) << 1) + 1, ((-1) << 2) + 1, ((-1) << 3) + 1, ((-1) << 4) + 1,
  ((-1) << 5) + 1, ((-1) << 6) + 1, ((-1) << 7) + 1, ((-1) << 8) + 1,
  ((-1) << 9) + 1, ((-1) << 10) + 1, ((-1) << 11) + 1, ((-1) << 12) + 1,
  ((-1) << 13) + 1, ((-1) << 14) + 1, ((-1) << 15) + 1 };

DInt32 DHuffTable::Extend(DInt32 readVal, DInt32 enVal)
{
    DAssert(enVal <= 11);
    DAssert(readVal <= 2047);

    DBool bSmall = readVal < extend_test[enVal];
    if (bSmall) {
        return readVal + extend_offset[enVal];
    }
    else {
        return readVal;
    }
}


/////////////////////////////////////////////////////////////////////////
// DHuffman

// 统计一个字符串中，每个字符出现的次数
DData* DHuffman::CountChar(const DStringA& str)
{
    DUInt32 counter[256] = {};
    DData* pDataRet = DData::CreateDData(256);
    DUInt32 len = str.GetDataLength();
    for (DUInt32 i = 0; i < len; i++) {
        DUInt8 c = str.GetAt(i);
        counter[c] ++;
    }
    for (DUInt32 i = 0; i < 256; i++) {
        if (counter[i] != 0) {
            pDataRet->AddUInt32(i, counter[i]);
        }
    }
    return pDataRet;
}


// 合并频率表中，两个最小的频率项，并更新编码表中的编码长度
static DVoid Merge2Freq(CodeLenItem* cl, DUInt32 clLen, DUInt32* fq, DUInt32 m1, DUInt32 m2) {
    fq[m1] = fq[m1] + fq[m2];
    fq[m2] = 65535;
    for (DUInt32 i = 0; i < clLen; i++) {
        if (cl[i].freqTableIndex == m2) {
            cl[i].codeLen++;
            cl[i].freqTableIndex = m1;
        }
        else if (cl[i].freqTableIndex == m1) {
            cl[i].codeLen++;
        }
    }
}


// 给定每个字符的出现次数，生成每个字符的CodeLen
DData* DHuffman::GenCodeLen(DData* pFrequencyTable)
{
    // Init codelenArr and freqArr
    DIntArray intArr = pFrequencyTable->Keys();
    DUInt32 size = intArr.GetSize();
    CodeLenItem* codelenArr = (CodeLenItem*)DALLOC(sizeof(CodeLenItem) * size);
    DUInt32* freqArr = (DUInt32*)DALLOC(sizeof(DUInt32) * size);
    if (codelenArr == nullptr || freqArr == nullptr) return nullptr;

    for (DUInt32 i = 0; i < size; i++) {
        codelenArr[i].charVal = intArr.GetAt(i);
        codelenArr[i].codeLen = 0;
        codelenArr[i].freqTableIndex = i;
        freqArr[i] = pFrequencyTable->GetUInt32(codelenArr[i].charVal);
    }

    // Combine with Find2Min
    for (DUInt32 i = 0; i < size - 1; i++) {
        DUInt32 m1 = 0, m2 = 0;
        Find2Min((DInt32*)freqArr, size, m1, m2);
        if (m1 > m2) {
            Merge2Freq(codelenArr, size, freqArr, m2, m1);
        }
        else {
            Merge2Freq(codelenArr, size, freqArr, m1, m2);
        }
    }

    DData* pData = DData::CreateDData(size);
    for (DUInt32 i = 0; i < size; i++) {
        pData->AddUInt8(codelenArr[i].charVal, codelenArr[i].codeLen);
    }

    DFREE(codelenArr);
    DFREE(freqArr);

    return pData;
}


// 给定编码长度表，得到 lengthCount 表
DData* DHuffman::GenLengthCount(DData* pCodeLenTable)
{
    DIntArray intArr = pCodeLenTable->Keys();
    DUInt32 size = intArr.GetSize();
    DUInt32 lengthCount[33] = {};
    for (DUInt32 i = 0; i < size; i++) {
        DUInt8 length = pCodeLenTable->GetUInt8(intArr.GetAt(i));
        if (length > 32) return nullptr;
        lengthCount[length] ++;
    }

    DData* pData = DData::CreateDData(size);
    for (DUInt32 i = 1; i <= 32; i++) {
        pData->AddUInt8(i, lengthCount[i]);
    }
    return pData;
}


// 限制最长编码长度为16的编码
DData* DHuffman::Limit16CodeLen(DData* pLenCountTable)
{
    DUInt8 clArr[33] = {};
    DIntArray intArr = pLenCountTable->Keys();
    DUInt32 size = intArr.GetSize();
    for (DUInt32 i = 0; i < size; i++) {
        clArr[intArr.GetAt(i)] = pLenCountTable->GetUInt8(intArr.GetAt(i));
    }

    for (DUInt32 i = 32; i >= 17; i--) {
        while (clArr[i] != 0) {
            DUInt32 j = i - 2;
            while (clArr[j] == 0) j--;
            // Replace a tree node with a value
            clArr[i] = clArr[i] - 2;
            clArr[i - 1] = clArr[i - 1] + 1;
            // Replace a value with a tree node
            clArr[j + 1] = clArr[j + 1] + 2;
            clArr[j] = clArr[j] - 1;
        }
    }

    DData* pData = DData::CreateDData(size);
    for (DUInt32 i = 1; i <= 16; i++) {
        pData->AddUInt8(i, clArr[i]);
    }
    return pData;
}


static DVoid SortByCodeLen(CodeLenItem* cl, DUInt32 clLen)
{
    CodeLenItem temp = {};
    for (int i = clLen - 1; i > 0; i--) {
        for (int j = 0; j < i; j++) {
            if (cl[j].codeLen > cl[j + 1].codeLen) {
                temp = cl[j];
                cl[j] = cl[j + 1];
                cl[j + 1] = temp;
            }
        }
    }
}

static DVoid PrintCodeLen(CodeLenItem* cl, DUInt32 clLen) {
    for (DUInt32 i = 0; i < clLen; i++) {
        DPrintf("%c: %d\n", cl[i].charVal, cl[i].codeLen);
    }
}

// 给定编码长度表，输出每个字符的编码
DData* DHuffman::GenCodec(DData* pCodeLenTable)
{
    DIntArray intArr = pCodeLenTable->Keys();
    DUInt32 size = intArr.GetSize();
    CodeLenItem* codelenArr = (CodeLenItem*)DALLOC(sizeof(CodeLenItem) * size);
    DMemZero(codelenArr, sizeof(CodeLenItem) * size);
    for (DUInt32 i = 0; i < size; i++) {
        codelenArr[i].charVal = intArr.GetAt(i);
        codelenArr[i].codeLen = pCodeLenTable->GetUInt8(codelenArr[i].charVal);
    }
    // Sort CodeLenItem
    //PrintCodeLen(codelenArr, size);
    SortByCodeLen(codelenArr, size);
    //PrintCodeLen(codelenArr, size);

    // Gen codec
    DUInt32 huffCodeCounter = 0;
    DUInt32 codeLenCounter = 1;
    for (DUInt32 i = 0; i < size;) {
        if (codelenArr[i].codeLen == codeLenCounter) {
            codelenArr[i++].huffCodec = huffCodeCounter;
            huffCodeCounter = huffCodeCounter + 1;
        }
        else {
            huffCodeCounter = huffCodeCounter << 1;
            codeLenCounter = codeLenCounter + 1;
        }
    }

    // Output
    DData* pData = DData::CreateDData(size);
    for (DUInt32 i = 0; i < size; i++) {
        pData->AddUInt32(codelenArr[i].charVal, codelenArr[i].huffCodec);
    }

    DFREE(codelenArr);
    return pData;
}

// 给定编码长度表，字符数组，返回每个字符的编码长度
DData* DHuffman::ExpandCodeLen(DData* pLengthCountTable, DBuffer charArr)
{
    DUInt8 codeLen[256] = {};
    DIntArray intArr = pLengthCountTable->Keys();
    DUInt32 size = intArr.GetSize();
    DUInt8 index = 0;
    for (DUInt32 i = 0; i < size; i++) {
        DUInt8 count = pLengthCountTable->GetUInt8(intArr.GetAt(i));
        for (DUInt32 j = 0; j < count; j++) {
            codeLen[charArr.GetAt(index)] = i;
            index++;
        }
    }

    DData* pData = DData::CreateDData(size);
    for (DUInt32 i = 0; i < 256; i++) {
        pData->AddUInt8(i, codeLen[i]);
    }
    return pData;
}


// 给定一个int数组，返回其中最小的2个数的下标，若有多个，则靠近数组头部
DVoid DHuffman::Find2Min(DInt32* arr, DUInt32 arrLen, DUInt32& min1, DUInt32& min2)
{
    DInt32 m1 = INT_MAX, m2 = INT_MAX;
    for (DUInt32 i = 0; i < arrLen; i++) {
        if (m1 > arr[i]) {
            m2 = m1;
            min2 = min1;
            m1 = arr[i];
            min1 = i;
        }
        else if (m2 > arr[i]) {
            m2 = arr[i];
            min2 = i;
        }
    }
}


// 给定Huffman编码表，解Buffer
DBuffer DHuffman::Decode(DBuffer clArr, DBuffer charArr, DBuffer cBuffer)
{
    // Code Length
    CodeLenItem codeItem[256] = {};
    DUInt32 index = 0;
    for (DUInt32 i = 0; i < 16; i++) {
        DUInt8 count = clArr.GetAt(i);
        for (DUInt32 j = 0; j < count; j++) {
            codeItem[index].charVal = charArr.GetAt(index);
            codeItem[index].codeLen = i;
            index++;
        }
    }

    // Gen codec
    DUInt32 huffCodeCounter = 0;
    DUInt32 codeLenCounter = 1;
    for (DUInt32 i = 0; i < index;) {
        if (codeItem[i].codeLen == codeLenCounter) {
            codeItem[i++].huffCodec = huffCodeCounter;
            huffCodeCounter = huffCodeCounter + 1;
        }
        else {
            huffCodeCounter = huffCodeCounter << 1;
            codeLenCounter = codeLenCounter + 1;
        }
    }

    // Fill decode Table
    DUInt16 MinCode[17] = {};
    DUInt16 MaxCode[17] = {};
    DUInt8 FirstData[17] = {};
    DUInt8 curLen = 0;
    DUInt8 curIndex = 1;
    for (DUInt32 i = 0; i < charArr.GetSize(); i++) {
        if (codeItem[i].codeLen > curLen) {
            MinCode[curIndex] = codeItem[i].huffCodec;
            FirstData[curIndex] = codeItem[i].charVal;
            MaxCode[curIndex] = codeItem[i].huffCodec;
            curLen = codeItem[i].codeLen;
            curIndex++;
        }
        else {
            MaxCode[curIndex - 1] = codeItem[i].huffCodec;
        }
    }

    DGrowBuffer gb;
    DBitStream bs(cBuffer);
    bs.SetMode(DBSMODE_START_BIT7_FIRST_MOST);
    DUInt32 code = 0;
    DUInt32 codeLength = 1;
    DBool hasData = true;
    while (hasData) {
        while (codeLength <= 16) {
            code = code << 1;
            DBool b;
            hasData = bs.ReadBit(&b);
            code = code | (b ? 1 : 0);
            codeLength = codeLength + 1;
            if (code <= MaxCode[codeLength]) {
                DUInt32 index = FirstData[codeLength] + code - MinCode[codeLength];
                gb.AddUInt8(charArr.GetAt(index - 1));
                break;
            }
        }
    }

    DBuffer bufRet = gb.Finish();
    return bufRet;
}
