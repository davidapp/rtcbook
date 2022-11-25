#include "DTestBuffer.h"
#include "Base/DTest.h"
#include "Base/DBuffer.h"

DVoid DTestBuffer::Test()
{
    DTestBuffer::TestBasic();
    DTestBuffer::TestConv();
}

DVoid DTestBuffer::TestBasic()
{
    ClearResult();

    // DBuffer 只在栈上维护一个指针
    DEXPECT_EQ_SIZE_T(DSizeOf(DBuffer), DSizeOf(DVoid*));

    // 空 Buffer
    DBuffer nullbuf(nullptr, 0);
    DEXPECT_TRUE(nullbuf.GetBuf() == DBuffer::GetNullBuffer().GetBuf());
    DEXPECT_EQ_INT(nullbuf.GetSize(), 0);

    // 普通构造
    DBuffer b(10);
    b.FillWithRandom();
    DEXPECT_TRUE(b.GetBuf() != nullptr);
    DEXPECT_EQ_INT(b.GetSize(), 10);

    // operator=
    DBuffer b2 = b;
    DEXPECT_TRUE(b2.GetBuf() == b.GetBuf());
    DEXPECT_EQ_INT(b2.GetSize(), 10);

    // 拷贝构造
    DBuffer b3(b2);
    DEXPECT_TRUE(b3.GetBuf() == b2.GetBuf());
    DEXPECT_EQ_INT(b3.GetSize(), 10);

    // 写时拷贝
    b3.SetAt(2, 255);
    DEXPECT_TRUE(b3.GetBuf() != b2.GetBuf());
    DEXPECT_EQ_INT(b3.GetAt(2), 255);

    TestResult("DBuffer::TestBasic");
}

DVoid DTestBuffer::TestConv()
{
    ClearResult();

    DBuffer buf(10);
    buf.FillWithRandom();
    DStringA str = buf.ToHexString();
    DBuffer buf2;
    buf2.InitWithHexString(str);
    DStringA str2 = buf2.ToHexString();
    DEXPECT_TRUE(str.Compare(str2) == 0);

    DStringA str64 = buf.ToBase64String();
    DBuffer buf642;
    buf642.InitWithBase64String(str64);
    DStringA str642 = buf642.ToBase64String();
    DEXPECT_TRUE(buf642.GetSize() == 10);
    DEXPECT_TRUE(str64.Compare(str642) == 0);

    TestResult("DBuffer::TestConv");
}


DVoid DTestGrowBuffer::Test()
{
    ClearResult();

    DGrowBuffer gb;
    gb.AddUInt8(1);
    gb.AddUInt16(2);
    gb.AddUInt32(3);
    gb.AddUInt64(4);
    gb.AddFloat(5);
    gb.AddDouble(6);
    gb.AddStringA(DStringA("12345"));
    gb.AddString(DString(u"67890"));
    DBuffer buf(10);
    buf.FillWithRandom();
    gb.AddBuffer(buf);
    DBuffer all = gb.Finish();
    DEXPECT_TRUE(all.GetSize() == 1 + 2 + 4 + 8 + 4 + 8 + 4 + 5 + 4 + 10 + 4 + 10);

    DReadBuffer rb(all);
    DEXPECT_TRUE(rb.ReadUInt8() == 1);
    DEXPECT_TRUE(rb.ReadUInt16() == 2);
    DEXPECT_TRUE(rb.ReadUInt32() == 3);
    DEXPECT_TRUE(rb.ReadUInt64() == 4);
    DEXPECT_TRUE(rb.ReadFloat() == 5);
    DEXPECT_TRUE(rb.ReadDouble() == 6);
    DEXPECT_TRUE(rb.ReadStringA() == "12345");
    DEXPECT_TRUE(rb.ReadString() == u"67890");
    DBuffer bufRet = rb.ReadBuffer();
    DAssert(buf == bufRet);

    TestResult("DGrowBuffer::Test");
}



DVoid DTestReadBuffer::Test()
{
    ClearResult();

    DGrowBuffer gb;
    gb.AddUInt8(1);
    gb.AddUInt16(2);
    gb.AddUInt32(3);
    gb.AddUInt64(4);
    gb.AddFloat(5);
    gb.AddDouble(6);
    gb.AddStringA(DStringA("12345"));
    gb.AddString(DString(u"67890"));
    DBuffer buf(10);
    buf.FillWithRandom();
    gb.AddBuffer(buf);
    DBuffer all = gb.Finish();
    DEXPECT_TRUE(all.GetSize() == 1 + 2 + 4 + 8 + 4 + 8 + 4 + 5 + 4 + 10 + 4 + 10);

    DReadBuffer rb(all);
    DEXPECT_TRUE(rb.ReadUInt8() == 1);
    DEXPECT_TRUE(rb.ReadUInt16() == 2);
    DEXPECT_TRUE(rb.ReadUInt32() == 3);
    DEXPECT_TRUE(rb.ReadUInt64() == 4);
    DEXPECT_TRUE(rb.ReadFloat() == 5);
    DEXPECT_TRUE(rb.ReadDouble() == 6);
    DEXPECT_TRUE(rb.ReadStringA() == "12345");
    DEXPECT_TRUE(rb.ReadString() == u"67890");
    DBuffer bufRet = rb.ReadBuffer();
    DEXPECT_TRUE(buf == bufRet);

    TestResult("DReadBuffer::Test");
}

