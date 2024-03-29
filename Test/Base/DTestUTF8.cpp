﻿#include "DTestUTF8.h"
#include "Base/DTest.h"
#include "Base/DUTF8.h"

DVoid DTestUTF8::Test()
{
    DTestUTF8::TestSimple();
    DTestUTF8::TestConvAll();
}

DVoid DTestUTF8::TestSimple()
{
    ClearResult();

    //u16 "哈哈你好123"
    //c8 54 c8 54 60 4f 7d 59 31 00 32 00 33 00 00 00
    DWChar strU[] = { 0x54c8, 0x54c8, 0x4f60, 0x597d, 0x0031, 0x0032, 0x0033, 0x0000 };
    DUInt32 nCount = DUTF8::UTF8Length16((DUInt16*)strU, 14);
    DEXPECT_EQ(nCount, 15);

    DByte strU8[] = {0xe5,0x93,0x88,0xe5,0x93,0x88,0xe4,0xbd,0xa0,0xe5,0xa5,0xbd,0x31,0x32,0x33,0};
    
    std::string strA1((DChar*)strU8, nCount);
    std::string strA2 = DUTF8::UCS2ToUTF8((DUInt16*)strU, 14);
    DEXPECT_EQ_STRING(strA1.c_str(), strA2.c_str(), nCount);
    
    DBool bRes = false;
    bRes = DUTF8::isUTF8((DByte*)strA1.c_str(), (DUInt32)strA1.size());
    DEXPECT_TRUE(bRes);

    bRes = DUTF8::isUTF8((DByte*)strA2.c_str(), (DUInt32)strA2.size());
    DEXPECT_TRUE(bRes);

    DUInt32 nCharCount = DUTF8::GetUTF8CharCount((DByte*)strA2.c_str(), (DUInt32)strA2.size());
    DEXPECT_EQ(nCharCount, 7);

    DWChar* pStr16 = new DWChar[nCharCount + 1];
    DUInt32 nCount16 = DUTF8::UTF8ToUCS2((DByte*)strA2.c_str(), (DUInt32)strA2.size(), pStr16);
    DEXPECT_EQ(nCount16, 7);
    DEXPECT_EQ_STRING(pStr16, strU, 14);
    delete[] pStr16;

    ShowResult();
}

DVoid DTestUTF8::TestConvAll()
{
    ClearResult();

#if defined(BUILD_FOR_WINDOWS)

    DChar u8code[6] = { 0 };
    DChar u8code2[6] = { 0 };
    DChar unicode[2] = { 0 };
    DChar unicode2[2] = { 0 };

    DInt32 total1 = 0, total2 = 0, total3 = 0, total0 = 0;
    DInt32 wrong = 0;

    //Test for All UCS2 Chars
    for (DInt32 i = 0x0000; i <= 0xFFFF; i++)
    {
        unicode[0] = (DByte)(i & 0x00FF);
        unicode[1] = (DByte)(i >> 8);
        DInt32 nWrite = ::WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)unicode, 1, (LPSTR)u8code, 6, NULL, 0);
        DInt32 nWrite2 = DUTF8::UTF8Encode16(*(DUInt16*)unicode, (DByte*)u8code2);
        if (nWrite == 3)
        {
            DEXPECT_EQ_INT(nWrite2, 3);
            DEXPECT_TRUE((u8code[0] == u8code2[0]) && (u8code[1] == u8code2[1]) && (u8code[2] == u8code2[2]));
            if (u8code[0] == u8code2[0] && u8code[1] == u8code2[1] && u8code[2] == u8code2[2])
            {
                total3++;
            }
            else
            {
                //DPrintf("0x%02x ", i); //0xEFBFBD
                wrong++;
            }
        }
        else if (nWrite == 2)
        {
            DEXPECT_EQ_INT(nWrite2, 2);
            DEXPECT_TRUE((u8code[0] == u8code2[0]) && (u8code[1] == u8code2[1]));
            total2++;
        }
        else if (nWrite == 1)
        {
            DEXPECT_EQ_INT(nWrite2, 1);
            DEXPECT_TRUE(u8code[0] == u8code2[0]);
            total1++;
        }
        else
        {
            total0++;
        }
    }
    DEXPECT_EQ_INT(wrong, 0);
    DEXPECT_EQ_INT(total0 + total1 + total2 + total3, 65536);   //测试字符总数
    DEXPECT_EQ_INT(total0, 0);      //异常字符
    DEXPECT_EQ_INT(total1, 128);    //1字节 UTF8字符
    DEXPECT_EQ_INT(total2, 1920);   //2字节 UTF8字符
    DEXPECT_EQ_INT(total3, 63488);  //3字节 UTF8字符
    // 测试结果将完全和 Windows的转换函数对齐
#else
    printf("DTestUTF8::TestConvAll() can only run on Windows\n");
#endif

    ShowResult();
}

/*
wrong=2048 0xD800-0xDFFF UTF-16 surrogate section
first byte 0xD800-0xDBFF=1024
second byte 0xDC00-0xDFFF=1024
1024*1024=1048576 total
63488(3:61440 2:1920 1:128 0:0) Unicode Char Tested!

wrong=0
65536(3:63488 2:1920 1:128 0:0) Unicode Char Tested
*/
