//
//  DDeflate.cpp
//
//  Created by Dai Wei(bmw.dai@gmail.com) on 02/14/2019 for Dream.
//  Copyright 2019. All rights reserved.
//

#include "DDeflate.h"
#include "Base/DMemAlloc.h"
#include "Base/DStdLib.h"
#include "Base/DBitStream.h"
#include "Base/DString.h"
#include "Data/DArray.h"

//////////////////////////////////////////////////////////////////////
// DDeflate
//
// 解析攻略
// https://www.cnblogs.com/esingchan/p/3958962.html
// RFC详细文档
// https://datatracker.ietf.org/doc/rfc1951/
// ZLIB官网
// http://www.zlib.net/
// ZLIB Benchmark
// http://www.htslib.org/benchmarks/zlib.html

#define DMAX_CCL_LEN 19
#define DMAX_CL1_LEN 256+1+29	//286
#define DMAX_CL2_LEN 30

// 8 Bytes
typedef struct tagHuff {
    DUInt16 ch;
    DUInt16 cl;
    DUInt32 huff;
    DUInt64 ToUInt64()
    {
        DUInt64 ret = huff;
        ret <<= 32;
        DUInt32 low = ch;
        low <<= 16;
        low += cl;
        ret += low;
        return ret;
    }
    DVoid FromUInt64(DUInt64 t)
    {
        huff = t >> 32;
        ch = (t >> 16) & 0xffff;
        cl = t & 0xffff;
    }
}DHuff;

// Huffman 表3，用来解码 Huffman1 和 Huffman2 表
DUInt32 ccl[DMAX_CCL_LEN] = { 0 };
DUInt32 ccl_mat[DMAX_CCL_LEN] = { 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15 };
DHuff ccl_huffman[DMAX_CCL_LEN];

// Huffman1 表
DUInt32 cl1[DMAX_CL1_LEN] = { 0 };
DHuff cl1_huffman[DMAX_CL1_LEN];

// Huffman2 表
DUInt32 cl2[DMAX_CL2_LEN] = { 0 };
DHuff cl2_huffman[DMAX_CL2_LEN];

// 临时数据结构，用来统计与生成码表
#define DNUM_CODELEN 16
DUInt32 bl_count[DNUM_CODELEN]; // 0-15
DUInt32 next_code[DNUM_CODELEN]; // 0-15

void init()
{
    for (int i = 0; i < DMAX_CCL_LEN; i++) ccl[i] = 0;
    for (int i = 0; i < DMAX_CL1_LEN; i++) cl1[i] = 0;
    for (int i = 0; i < DMAX_CL2_LEN; i++) cl2[i] = 0;
}

void print_cl(DHuff* pArray, DUInt32 size)
{
    for (int i = 0; i < (int)size - 1; i++)
    {
        DPrintf("%u(%u),", pArray[i].cl, pArray[i].ch);
    }
    DPrintf("%u(%u)\n", pArray[size - 1].cl, pArray[size - 1].ch);
}

void print_code(DHuff* pArray, DUInt32 size)
{
    for (int i = 0; i < (int)size; i++)
    {
        if (pArray[i].cl != 0)
        {
            DBitSet bs(pArray[i].huff, pArray[i].cl);
            for (int j = (int)pArray[i].cl - 1; j >= 0; j--)
            {
                if (bs.TestBit(j)) DPrintf("1");
                else DPrintf("0");
            }
            DPrintf("[%u] <=> %u\n", pArray[i].cl, pArray[i].ch);
        }
    }
}

void genhuff(DHuff* pArray, DUInt32 size)
{
    for (int i = 0; i < DNUM_CODELEN; i++)
    {
        bl_count[i] = 0;
        next_code[i] = 0;
    }

    for (int i = 0; i < (int)size; i++)
    {
        bl_count[pArray[i].cl]++;
    }
    bl_count[0] = 0;

    DUInt32 code = 0;
    for (int bits = 1; bits < DNUM_CODELEN; bits++)
    {
        code = (code + bl_count[bits - 1]) << 1;
        next_code[bits] = code;
    }

    for (int i = 0; i < (int)size; i++)
    {
        DUInt32 len = pArray[i].cl;
        if (len != 0)
        {
            pArray[i].huff = next_code[len];
            next_code[len]++;
        }
    }
}

int findmatch(DUInt32 v, DUInt32 l)
{
    for (int i = 0; i < DMAX_CCL_LEN; i++)
    {
        if (v == ccl_huffman[i].huff && l == ccl_huffman[i].cl)
        {
            return ccl_huffman[i].ch;
        }
    }
    return -1;
}

int findmatch_cl1(DUInt32 v, DUInt32 l)
{
    for (int i = 0; i < DMAX_CL1_LEN; i++)
    {
        if (v == cl1_huffman[i].huff && l == cl1_huffman[i].cl)
        {
            return cl1_huffman[i].ch;
        }
    }
    return -1;
}

int findmatch_cl2(DUInt32 v, DUInt32 l)
{
    for (int i = 0; i < DMAX_CL2_LEN; i++)
    {
        if (v == cl2_huffman[i].huff && l == cl2_huffman[i].cl)
        {
            return cl2_huffman[i].ch;
        }
    }
    return -1;
}

// 4 Bytes
typedef struct tagDistRange
{
    DUInt16 bit;
    DUInt16 start;
}DDistRange;

DDistRange distTable[30] = {
    {0,1},{0,2},{0,3},{0,4},{1,5},{1,7},
    {2,9},{2,13},{3,17},{3,25},{4,33},{4,49},
    {5,65},{5,97},{6,129},{6,193},{7,257},{7,385},
    {8,513},{8,769},{9,1025},{9,1537},{10,2049},{10,3073},
    {11,4097},{11,6145},{12,8193},{12,12289},{13,16385},{13,24577}
};

// 4 Bytes
typedef struct tagLengthRange
{
    DUInt16 bit;
    DUInt16 startlen;
}DLengthRange;

DLengthRange lenTable[29] = {
    {0,3},{0,4},{0,5},{0,6},{0,7},{0,8},
    {0,9},{0,10},{1,11},{1,13},{1,15},{1,17},
    {2,19},{2,23},{2,27},{2,31},{3,35},{3,43},
    {3,51},{3,59},{4,67},{4,83},{4,99},{4,115},
    {5,131},{5,163},{5,195},{5,227},{0,258}
};

DData* DDeflate::Load(DBuffer bufDeflate)
{
    init();

    DData* pRet = DData::CreateDData(20);

    DBitStream bss(bufDeflate);
    bss.SetMode(DBSMODE_FOR_DEFLATE); // use default mode for DDeflate

    // first bit   BFINAL is set if and only if this is the last block of the data set.
    DBool bFinal;
    bss.ReadBit(&bFinal);
    pRet->AddBool(DDEFLATE_1BIT_FINAL, bFinal);

    // next 2 bits   BTYPE
    // BTYPE specifies how the data are compressed, as follows :
    // 00 - no compression
    // 01 - compressed with fixed Huffman codes
    // 10 - compressed with dynamic Huffman codes
    // 11 - reserved(error)
    DBitSet bs;
    DBool bOK = bss.ReadBits(2, &bs);
    DUInt8 bType = bs.GetFinalValue();
    pRet->AddUInt8(DDEFLATE_2BIT_TYPE, bType);

    // 5 Bits : HLIT, # of Literal / Length codes - 257 (257 - 286)
    bs.Reset();
    bss.ReadBits(5, &bs);	// 1-29
    DUInt16 hlit = bs.GetValue() + 257;
    pRet->AddUInt16(DDEFLATE_5BIT_HLIT, hlit);

    // 5 Bits: HDIST, # of Distance codes - 1 (1 - 32)
    bs.Reset();
    bss.ReadBits(5, &bs);	//1-30
    DUInt8 hdist = bs.GetValue() + 1;
    pRet->AddUInt8(DDEFLATE_5BIT_HDIST, hdist);

    // 4 Bits: HCLEN, # of Code Length codes - 4 (4 - 19)
    // 表示后面有多少个 3bit 的 CodeLength 编码
    bs.Reset();
    bss.ReadBits(4, &bs);	//0-15
    DUInt32 hclen = bs.GetValue() + 4;
    pRet->AddUInt8(DDEFLATE_4BIT_HCLEN, hclen);

    for (DUInt32 i = 0; i < hclen; i++)
    {
        bs.Reset();
        bss.ReadBits(3, &bs); // 0-7
        DUInt32 cclitem = bs.GetValue();
        ccl[ccl_mat[i]] = cclitem;
    }

    DArray* pCodeLengthArray = DArray::CreateDArray(20);
    for (DUInt32 i = 0; i < DMAX_CCL_LEN; i++) // 19项 CCL
    {
        ccl_huffman[i].ch = i;
        ccl_huffman[i].cl = ccl[i];	// 每个编码长度的个数限制在 0-7
        pCodeLengthArray->AddUInt32(ccl[i]);
    }
    pRet->AddArray(DDEFLATE_ARRAY_CCL, pCodeLengthArray);
    pCodeLengthArray->Release();

    DStringA strHuff3 = CodeLengthToString("HuffTable3", pCodeLengthArray);
    strHuff3.Print();

    // 这个数组的含义为，对0-18这19个数字进行Huffman编码，后面为其Huffman编码的bit数
    // 3、5、5、5、3、2、2、0、0、0、0、0、0、0、0、0、0、5、3
    // 0 需要 3bit
    // 1 需要 5bit
    // ...
    // 18 需要 3bit
    // 有了这个数组，我们就可以生成Huffman编码表

    DArray* pHuffTable3 = GenHuffCodec(pCodeLengthArray);
    pRet->AddArray(DDEFLATE_ARRAY_CCL_CODEC, pHuffTable3);
    DStringA strHuffTable3 = HuffTableToString("HuffTable3", pHuffTable3);
    strHuffTable3.Print();

    // 使用 Huffman 表3，解码 Huffman 表1，一共 HLIT+257(256文本字符+结束字符) 项
    // HLIT 5位(1-29用来表示长度的区间lenTable)
    DUInt32 litcounter = 0;
    DUInt32 lastcl = 0;
    bs.Reset();
    DBool btemp;
    DArray* pCL1_SEQ = DArray::CreateDArray(40);
    while (!bss.IsFinish())
    {
        // 每次读一位
        bss.ReadBit(&btemp);

        // 由于这里是Big-Endian的，倒过来存储
        if (btemp)
        {
            bs.AddTrueToTail();
        }
        else
        {
            bs.AddFalseToTail();
        }

        // 使用 Huffman 表3 看看是否匹配
        DInt32 find = FindMatch(pHuffTable3, bs);
        if (find != -1)
        {
            // 如果已经找到
            if (find >= 0 && find <= 15)
            {
                // 0-15 为 literal 本身，直接存储
                cl1[litcounter] = find;
                litcounter++;
                lastcl = find;
            }
            else if (find == 16)
            {
                // 16 为 后面2位+3，重复这么多次最后出现的字符
                DBitSet bs2;
                bs2.Reset();
                bss.ReadBits(2, &bs2);
                DUInt32 rcount = bs2.GetValue() + 3;
                for (DUInt32 i = 0; i < rcount; i++)
                {
                    cl1[litcounter] = lastcl;
                    litcounter++;
                }
            }
            else if (find == 17)
            {
                // 17 为 后面3位+3，重复这么多次0
                DBitSet bs2;
                bs2.Reset();
                bss.ReadBits(3, &bs2);
                DUInt32 rcount = bs2.GetValue() + 3;
                for (DUInt32 i = 0; i < rcount; i++)
                {
                    cl1[litcounter] = 0;
                    litcounter++;
                }
            }
            else if (find == 18)
            {
                // 18 为 后面7位+11，重复这么多次0
                DBitSet bs2;
                bs2.Reset();
                bss.ReadBits(7, &bs2);
                DUInt32 rcount = bs2.GetValue() + 11;
                for (DUInt32 i = 0; i < rcount; i++)
                {
                    cl1[litcounter] = 0;
                    litcounter++;
                }
            }
            pCL1_SEQ->AddUInt32(find);
            bs.Reset();
        }
        else
        {
            if (bs.GetLen() > 7)
            {
                // 长度已经大于最大的，说明数据出错
                break;
            }
        }

        // 个数达到了就结束
        if (litcounter >= hlit)
        {
            break;
        }
    }
    //pRet->AddArray(DDEFLATE_ARRAY_CL1_SEQ, pCL1_SEQ);
    pCL1_SEQ->Release();

    // CL1的 CodeLengthArray
    DArray* pCL1Array = DArray::CreateDArray(DMAX_CL1_LEN);
    for (DUInt32 i = 0; i < DMAX_CL1_LEN; i++)
    {
        pCL1Array->AddUInt32(cl1[i]);
    }
    pRet->AddArray(DDEFLATE_ARRAY_CL1, pCL1Array);
    DStringA strHuff1 = CodeLengthToString("HuffTable1", pCL1Array);
    strHuff1.Print();
    pCL1Array->Release();

    // 生成 CL1的 Huffman表1
    DArray* pHuffTable1 = GenHuffCodec(pCL1Array);
    pRet->AddArray(DDEFLATE_ARRAY_CL1_CODEC, pHuffTable1);
    pHuffTable1->Release();
    DStringA strHuffTable1 = HuffTableToString("HuffTable1", pHuffTable1);
    strHuffTable1.Print();

    // 使用 Huffman 表3，解码 Huffman 表2
    // 获取 CL2的 序列
    DUInt32 distcounter = 0;
    DUInt32 lastdist = 0;
    bs.Reset();
    DArray* pCL2_SEQ = DArray::CreateDArray(40);
    while (!bss.IsFinish())
    {
        bss.ReadBit(&btemp);
        if (btemp)
        {
            bs.AddTrueToTail();
        }
        else
        {
            bs.AddFalseToTail();
        }

        DInt32 find = FindMatch(pHuffTable3, bs);
        if (find != -1)
        {
            if (find >= 0 && find <= 15)
            {
                cl2[distcounter] = find;
                distcounter++;
                lastdist = find;
            }
            else if (find == 16)
            {
                DBitSet bs2;
                bs2.Reset();
                bss.ReadBits(2, &bs2);
                DUInt32 rcount = bs2.GetValue() + 3;
                for (DUInt32 i = 0; i < rcount; i++)
                {
                    cl2[distcounter] = lastdist;
                    distcounter++;
                }
            }
            else if (find == 17)
            {
                DBitSet bs2;
                bs2.Reset();
                bss.ReadBits(3, &bs2);
                DUInt32 rcount = bs2.GetValue() + 3;
                for (DUInt32 i = 0; i < rcount; i++)
                {
                    cl2[distcounter] = 0;
                    distcounter++;
                }
            }
            else if (find == 18)
            {
                DBitSet bs2;
                bs2.Reset();
                bss.ReadBits(7, &bs2);
                DUInt32 rcount = bs2.GetValue() + 11;
                for (DUInt32 i = 0; i < rcount; i++)
                {
                    cl2[distcounter] = 0;
                    distcounter++;
                }
            }
            pCL2_SEQ->AddUInt32(find);
            bs.Reset();
        }
        else
        {
            if (bs.GetLen() > 7)
            {
                // 长度已经大于最大的，说明数据出错
                break;
            }
        }

        if (distcounter >= hdist)
        {
            break;
        }
    }

    //pRet->AddArray(DDEFLATE_ARRAY_CL2_SEQ, pCL2_SEQ);
    pCL2_SEQ->Release();

    DArray* pCL2Array = DArray::CreateDArray(DMAX_CL2_LEN);
    for (DUInt32 i = 0; i < DMAX_CL2_LEN; i++)
    {
        pCL2Array->AddUInt32(cl2[i]);
    }
    pRet->AddArray(DDEFLATE_ARRAY_CL2, pCL2Array);
    DStringA strHuff2 = CodeLengthToString("HuffTable2", pCL2Array);
    strHuff2.Print();
    pCL2Array->Release();

    DArray* pHuffTable2 = GenHuffCodec(pCL2Array);
    pRet->AddArray(DDEFLATE_ARRAY_CL2_CODEC, pHuffTable2);
    pHuffTable2->Release();
    DStringA strHuffTable2 = HuffTableToString("HuffTable2", pHuffTable2);
    strHuffTable2.Print();

    DBitStream gb(32 * 1024);	//32K
    bs.Reset();
    DArray* pArraySEQ = DArray::CreateDArray();
    DStringA strSeq, strTemp;
    while (!bss.IsFinish())
    {
        bss.ReadBit(&btemp);
        if (btemp)
        {
            bs.AddTrueToTail();
        }
        else
        {
            bs.AddFalseToTail();
        }

        // 先使用 Huffman表1 解码
        DInt32 find = FindMatch(pHuffTable1, bs);
        if (find != -1)
        {
            if (find >= 0 && find <= 255)
            {
                // 0-255 为 Literal
                strSeq.Format("[%d]", find);
                gb.AddUInt8(find);
                pArraySEQ->AddStringA(strSeq);
            }
            else if (find == 256)
            {
                // 256 为 Finish
                strSeq.Format("[end]");
                pArraySEQ->AddStringA(strSeq);
                break;
            }
            else if (find >= 257 && find <= 285)
            {
                // 257-285 为 Length 区间
                DUInt32 nIndex = find - 257;
                DLengthRange lenR = lenTable[nIndex];
                DUInt32 bitNeed = lenR.bit;
                D_UNUSED(bitNeed);
                DUInt32 lenValue = lenR.startlen;

                DUInt32 distValue = 0;
                if (lenR.bit != 0)
                {
                    DBitSet code;
                    code.Reset();
                    bss.ReadBits(lenR.bit, &code);
                    DUInt32 c = code.GetValue();
                    lenValue += c;
                }

                strSeq.Format("(%d, ", lenValue);

                // Length 后面跟着的是 distance，使用 Huffman表2 来解码
                DBitSet distSet;
                distSet.Reset();
                while (!bss.IsFinish())
                {
                    bss.ReadBit(&btemp);
                    if (btemp)
                    {
                        distSet.AddTrueToTail();
                    }
                    else
                    {
                        distSet.AddFalseToTail();
                    }

                    DInt32 find2 = FindMatch(pHuffTable2, distSet);
                    if (find2 >= 0 && find2 <= 29)
                    {
                        DDistRange distR = distTable[find2];
                        distValue = distR.start;
                        if (distR.bit != 0)
                        {
                            if (distR.bit == 8) {
                                int r = 0;
                            }
                            DBitSet code2;
                            code2.Reset();
                            bss.ReadBits(distR.bit, &code2);
                            DUInt32 c2 = code2.GetValue();
                            distValue += c2;
                        }
                        if (distValue > 33000)
                        {
                            break;
                        }
                        strTemp.Format("%d)", distValue);
                        strSeq += strTemp;
                        pArraySEQ->AddStringA(strSeq);
                        break;
                    }
                }
                gb.AddDup(distValue, lenValue);
            }
            bs.Reset();
        }
    }

    pRet->AddArray(DDEFLATE_ARRAY_CONTENT_SEQ, pArraySEQ);
    pArraySEQ->Release();
    DStringA strSEQ = ContentSeqToString(pArraySEQ);
    strSEQ.Print();

    DBuffer bufAll = gb.Finish();
    pRet->AddBuffer(DDEFLATE_BUFFER_CONTENT, bufAll);

    return pRet;
}

DArray* DDeflate::GenHuffCodec(DArray* pCLArray)
{
    // 16项清零
    for (DUInt32 i = 0; i < DNUM_CODELEN; i++)
    {
        bl_count[i] = 0;
        next_code[i] = 0;
    }

    DUInt32 uCLSize = pCLArray->GetSize();
    for (DUInt32 i = 0; i < uCLSize; i++)
    {
        bl_count[pCLArray->GetUInt32(i)]++;
    }
    bl_count[0] = 0;

    DUInt32 code = 0;
    for (DUInt32 bits = 1; bits < DNUM_CODELEN; bits++)
    {
        code = (code + bl_count[bits - 1]) << 1;
        next_code[bits] = code;
    }

    DArray* pArrayRet = DArray::CreateDArray(uCLSize);
    for (DUInt32 i = 0; i < uCLSize; i++)
    {
        DUInt32 len = pCLArray->GetUInt32(i);
        DHuff huf = {};
        if (len != 0)
        {
            huf.huff = next_code[len];
            huf.cl = len;
            next_code[len]++;
        }
        else
        {
            huf.huff = 0;
            huf.cl = 0;
        }
        huf.ch = i;
        pArrayRet->AddUInt64(huf.ToUInt64());
    }
    return pArrayRet;
}

DInt32 DDeflate::FindMatch(DArray* pHuffTable, DBitSet bs)
{
    DUInt32 uSize = pHuffTable->GetSize();
    for (DUInt32 i = 0; i < uSize; i++)
    {
        DUInt64 t = pHuffTable->GetUInt64(i);
        DHuff huf;
        huf.FromUInt64(t);
        if (bs.GetValue() == huf.huff && bs.GetLen() == huf.cl)
        {
            return huf.ch;
        }
    }
    return -1;
}

DStringA DDeflate::CodeLengthToString(DCStr arrName, DArray* pCLTable)
{
    DStringA strRet, strTemp;

    strTemp.Format("CodeLength for:%s\n", arrName);
    strRet += strTemp;

    DUInt32 uSize = pCLTable->GetSize();
    strTemp.Format("Value Range:(%d - %d)\n", 0, uSize - 1);
    strRet += strTemp;

    for (DUInt32 i = 0; i < uSize; i++)
    {
        DUInt32 cl = pCLTable->GetUInt32(i);
        if (cl != 0)
        {
            strTemp.Format("Value %2d: need %d bits\n", i, cl);
            strRet += strTemp;
        }
    }

    return strRet;
}

DStringA DDeflate::HuffTableToString(DCStr hufName, DArray* pHuffTable)
{
    DStringA strRet, strTemp;

    DUInt32 uSize = pHuffTable->GetSize();
    strTemp.Format("%s Size:%u\n", hufName, uSize);
    strRet += strTemp;

    for (DUInt32 i = 0; i < uSize; i++)
    {
        DUInt64 t = pHuffTable->GetUInt64(i);
        DHuff huf;
        huf.FromUInt64(t);
        if (huf.cl != 0)
        {
            strTemp.Format("%2d <=> ", huf.ch);
            strRet += strTemp;

            DBitSet bs(huf.huff, huf.cl);
            for (int j = (int)huf.cl - 1; j >= 0; j--)
            {
                if (bs.TestBit(j)) strRet += "1";
                else strRet += "0";
            }
            strTemp.Format("[%u]\n", huf.cl);
            strRet += strTemp;
        }
    }
    return strRet;
}

DStringA DDeflate::ContentSeqToString(DArray* pSEQTable)
{
    DStringA strRet;
    DUInt32 uSize = pSEQTable->GetSize();
    for (DUInt32 i = 0; i < uSize; i++)
    {
        DStringA strItem = pSEQTable->GetStringA(i);
        strRet += strItem;
        strRet += " ";
    }
    return strRet;
}

DBuffer DDeflate::UnZip(DBuffer bufDeflate)
{
    init();

    DBitStream bss(bufDeflate);
    bss.SetMode(DBSMODE_FOR_DEFLATE); // use default mode for DDeflate

    DBitSet bs;
    bss.ReadBits(3, &bs);
    DUInt32 header = bs.GetValue();
    //TODO See 00 or 01 or 10, we assume it is 01 for fixed Huffman codes
    D_UNUSED(header); // 1 01

    bs.Reset();
    bss.ReadBits(5, &bs);	//1-29
    DUInt32 hlit = bs.GetValue() + 257;

    bs.Reset();
    bss.ReadBits(5, &bs);	//1-30
    DUInt32 hdist = bs.GetValue() + 1;

    bs.Reset();
    bss.ReadBits(4, &bs);	//0-15
    DUInt32 hclen = bs.GetValue() + 4;

    for (DUInt32 i = 0; i < hclen; i++)
    {
        bs.Reset();
        bss.ReadBits(3, &bs);
        DUInt32 cclitem = bs.GetValue();
        ccl[ccl_mat[i]] = cclitem;
    }

    for (DUInt32 i = 0; i < DMAX_CCL_LEN; i++)
    {
        ccl_huffman[i].ch = i;
        ccl_huffman[i].cl = ccl[i];
    }

    genhuff(ccl_huffman, DMAX_CCL_LEN);

    DUInt32 litcounter = 0;
    DUInt32 lastcl = 0;
    bs.Reset();
    DBool btemp;
    while (!bss.IsFinish())
    {
        bss.ReadBit(&btemp);
        if (btemp)
        {
            bs.AddTrueToTail();
        }
        else
        {
            bs.AddFalseToTail();
        }

        DInt32 find = findmatch(bs.GetValue(), bs.GetLen());
        if (find != -1)
        {
            if (find >= 0 && find <= 15)
            {
                cl1[litcounter] = find;
                litcounter++;
                lastcl = find;
            }
            else if (find == 16)
            {
                DBitSet bs2;
                bs2.Reset();
                bss.ReadBits(2, &bs2);
                DUInt32 rcount = bs2.GetValue() + 3;
                for (DUInt32 i = 0; i < rcount; i++)
                {
                    cl1[litcounter] = lastcl;
                    litcounter++;
                }
            }
            else if (find == 17)
            {
                DBitSet bs2;
                bs2.Reset();
                bss.ReadBits(3, &bs2);
                DUInt32 rcount = bs2.GetValue() + 3;
                for (DUInt32 i = 0; i < rcount; i++)
                {
                    cl1[litcounter] = 0;
                    litcounter++;
                }
            }
            else if (find == 18)
            {
                DBitSet bs2;
                bs2.Reset();
                bss.ReadBits(7, &bs2);
                DUInt32 rcount = bs2.GetValue() + 11;
                for (DUInt32 i = 0; i < rcount; i++)
                {
                    cl1[litcounter] = 0;
                    litcounter++;
                }
            }
            bs.Reset();
        }
        else
        {
            if (bs.GetLen() > 7)
            {
                // 长度已经大于最大的，说明数据出错
                break;
            }
        }

        if (litcounter >= hlit) {
            break;
        }
    }

    for (DUInt32 i = 0; i < DMAX_CL1_LEN; i++)
    {
        cl1_huffman[i].ch = i;
        cl1_huffman[i].cl = cl1[i];
    }

    genhuff(cl1_huffman, DMAX_CL1_LEN);

    DUInt32 distcounter = 0;
    DUInt32 lastdist = 0;
    bs.Reset();
    while (!bss.IsFinish())
    {
        bss.ReadBit(&btemp);
        if (btemp)
        {
            bs.AddTrueToTail();
        }
        else
        {
            bs.AddFalseToTail();
        }

        DInt32 find = findmatch(bs.GetFinalValue(), bs.GetLen());
        if (find != -1)
        {
            if (find >= 0 && find <= 15)
            {
                cl2[distcounter] = find;
                distcounter++;
                lastdist = find;
            }
            else if (find == 16)
            {
                DBitSet bs2;
                bs2.Reset();
                bss.ReadBits(2, &bs2);
                DUInt32 rcount = bs2.GetValue() + 3;
                for (DUInt32 i = 0; i < rcount; i++)
                {
                    cl2[distcounter] = lastdist;
                    distcounter++;
                }
            }
            else if (find == 17)
            {
                DBitSet bs2;
                bs2.Reset();
                bss.ReadBits(3, &bs2);
                DUInt32 rcount = bs2.GetValue() + 3;
                for (DUInt32 i = 0; i < rcount; i++)
                {
                    cl2[distcounter] = 0;
                    distcounter++;
                }
            }
            else if (find == 18)
            {
                DBitSet bs2;
                bs2.Reset();
                bss.ReadBits(7, &bs2);
                DUInt32 rcount = bs2.GetValue() + 11;
                for (DUInt32 i = 0; i < rcount; i++)
                {
                    cl2[distcounter] = 0;
                    distcounter++;
                }
            }
            bs.Reset();
        }
        else
        {
            if (bs.GetLen() > 7)
            {
                // 长度已经大于最大的，说明数据出错
                break;
            }
        }
        if (distcounter >= hdist)
        {
            break;
        }
    }

    for (DUInt32 i = 0; i < DMAX_CL2_LEN; i++)
    {
        cl2_huffman[i].ch = i;
        cl2_huffman[i].cl = cl2[i];
    }

    genhuff(cl2_huffman, DMAX_CL2_LEN);

    DBitStream gb(32 * 1024);	//32K
    bs.Reset();
    while (!bss.IsFinish())
    {
        bss.ReadBit(&btemp);
        if (btemp)
        {
            bs.AddTrueToTail();
        }
        else
        {
            bs.AddFalseToTail();
        }

        DInt32 find = findmatch_cl1(bs.GetValue(), bs.GetLen());
        if (find != -1)
        {
            if (find >= 0 && find <= 255)
            {
                //Literal
                gb.AddUInt8(find);
            }
            else if (find == 256)
            {
                //Finish
                break;
            }
            else if (find >= 257 && find <= 285)
            {
                //Length
                DUInt32 nIndex = find - 257;
                DLengthRange lenR = lenTable[nIndex];
                DUInt32 bitNeed = lenR.bit;
                D_UNUSED(bitNeed);
                DUInt32 lenValue = lenR.startlen;

                DUInt32 distValue = 0;
                if (lenR.bit != 0)
                {
                    DBitSet code;
                    code.Reset();
                    bss.ReadBits(lenR.bit, &code);
                    DUInt32 c = code.GetValue();
                    lenValue += c;
                }
                //Next dist Using CL2
                DBitSet distSet;
                distSet.Reset();
                while (!bss.IsFinish())
                {
                    bss.ReadBit(&btemp);
                    if (btemp)
                    {
                        distSet.AddTrueToTail();
                    }
                    else
                    {
                        distSet.AddFalseToTail();
                    }

                    DInt32 find2 = findmatch_cl2(distSet.GetValue(), distSet.GetLen());
                    if (find2 >= 0 && find2 <= 29)
                    {
                        DDistRange distR = distTable[find2];
                        distValue = distR.start;
                        if (distR.bit != 0)
                        {
                            DBitSet code2;
                            code2.Reset();
                            bss.ReadBits(distR.bit, &code2);
                            DUInt32 c2 = code2.GetValue();
                            distValue += c2;
                        }
                        break;
                    }
                }
                gb.AddDup(distValue, lenValue);
            }
            bs.Reset();
        }
    }

    DBuffer bufRet = gb.Finish();
    //bufRet.Dump();
    return bufRet;
}
