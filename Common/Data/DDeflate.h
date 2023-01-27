//
//  DDeflate.h
//
//  Created by Dai Wei(bmw.dai@gmail.com) on 02/14/2019 for Dream.
//  Copyright 2019. All rights reserved.
//

#pragma once

#include "DConfig.h"
#include "DTypes.h"
#include "Base/DBuffer.h"
#include "Base/DBitStream.h"
#include "Data/DData.h"
#include "Data/DArray.h"

enum {
    DDEFLATE_1BIT_FINAL = 0,
    DDEFLATE_2BIT_TYPE = 1, // 00 不压缩 01 静态编码 10动态编码 11错误
    DDEFLATE_5BIT_HLIT = 2, // CL1序列的个数 = HLIT+257
    DDEFLATE_5BIT_HDIST = 3, // CL2序列的个数 = HDIST+1
    DDEFLATE_4BIT_HCLEN = 4, // CCL序列的个数 = HCLEN+4
    DDEFLATE_ARRAY_CCL = 5, // CCL数组 值为0-7，因为只需要表示0-18这19个数，7位可以表示128个数字
    DDEFLATE_ARRAY_CCL_CODEC = 6, // CCL Huffman编码表
    DDEFLATE_ARRAY_CL1_SEQ = 7, // CL1原始扫描信息
    DDEFLATE_ARRAY_CL1 = 8, // CL1数组 值为0-15，因为只需要表示0-285这286个数(literal/length)，16位可以表示65536个数字()
    DDEFLATE_ARRAY_CL1_CODEC = 9, // CL1 Huffman编码表
    DDEFLATE_ARRAY_CL2_SEQ = 10, // CL2原始扫描信息
    DDEFLATE_ARRAY_CL2 = 11, // CL2数组 值为0-15，因为只需要表示0-29这30个数(distance)，16位可以表示65536个数字
    DDEFLATE_ARRAY_CL2_CODEC = 12, // CL2 Huffman编码表
    DDEFLATE_ARRAY_CONTENT_SEQ = 13, // Deflate内容的编码，要么是literal(0-255) 要么是(len+distance) 256为结束
    DDEFLATE_BUFFER_CONTENT = 14 // 解码后的 Buffer
};

#define DBSMODE_FOR_DEFLATE DBSMODE_START_BIT0_SECOND_MOST

class DDeflate
{
public:
    static DData* Load(DBuffer bufDeflate);
    static DBuffer UnZip(DBuffer bufDeflate);

public:
    static DArray* GenHuffCodec(DArray* pCLArray);
    static DInt32  FindMatch(DArray* pHuffTable, DBitSet bs);

public:
    static DStringA CodeLengthToString(DCStr arrName, DArray* pCLTable);
    static DStringA HuffTableToString(DCStr hufName, DArray* pHuffTable);
    static DStringA ContentSeqToString(DArray* pSEQTable);
};
