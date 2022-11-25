//
//  DTest.h
//	Test Framework for Dream
//
//  Created by Dai Wei(bmw.dai@gmail.com) on 06/01/2018 for Dream.
//  Copyright 2018. All rights reserved.
//

#pragma once

#include "DConfig.h"
#include "DTypes.h"
#include "DXP.h"

static DInt32 test_count = 0;   // 用例总数
static DInt32 test_pass = 0;    // 用例通过数
static DBool  assert_mode = true;  // 是否打开 assert 模式

#define ClearResult() \
    test_count = 0; \
    test_pass = 0;

//Output diff when not equality
#define DEXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            printf("%s:%d: expect: " format " actual: " format D_LINES, __FILE__, __LINE__, expect, actual);\
            if (assert_mode) assert(0);\
        }\
    } while(0)

//Test Int
#define DEXPECT_EQ_INT(actual, expect) DEXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define DEXPECT_EQ_UINT(actual, expect) DEXPECT_EQ_BASE((expect) == (actual), expect, actual, "%u")

//Test Int64
#define DEXPECT_EQ_INT64(actual, expect) DEXPECT_EQ_BASE((expect) == (actual), expect, actual, "%lld")
#define DEXPECT_EQ_UINT64(actual, expect) DEXPECT_EQ_BASE((expect) == (actual), expect, actual, "%llu")

//Test Float
#define DEXPECT_EQ_FLOAT(actual, expect) DEXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17f")

//Test Double
#define DEXPECT_EQ_DOUBLE(actual, expect) DEXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")

//Test StringA
#define DEXPECT_EQ_STRING(actual, expect, alength) \
    DEXPECT_EQ_BASE(memcmp(expect, actual, alength) == 0, (DChar*)expect, (DChar*)actual, "%s")

//Test Buffer
#define DEXPECT_EQ_BUFFER(actual, expect, alength) \
    DEXPECT_EQ_BASE(memcmp(expect, actual, alength) == 0, expect, actual, "%s")

//Test SizeT
#if defined(TARGET_64BIT) && (TARGET_64BIT==1)
#define DEXPECT_EQ_SIZE_T(actual, expect) DEXPECT_EQ_BASE((expect) == (actual), (DSizeT)expect, (DSizeT)actual, "%llu")
#else
#define DEXPECT_EQ_SIZE_T(actual, expect) DEXPECT_EQ_BASE((expect) == (actual), (DSizeT)expect, (DSizeT)actual, "%lu")
#endif

//Test True
#define DEXPECT_TRUE(actual) DEXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")

//Test False
#define DEXPECT_FALSE(actual) DEXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")

//Test Great or Equal
#define DEXPECT_GE(a, b) DEXPECT_TRUE((a) >= (b))
#define DEXPECT_GT(a, b) DEXPECT_TRUE((a) >  (b))
#define DEXPECT_LE(a, b) DEXPECT_TRUE((a) <= (b))
#define DEXPECT_LT(a, b) DEXPECT_TRUE((a) <  (b))

//Test with small miss
#define DEXPECT_NEAR_EQ_INT(actual, expect, miss) \
    do {\
        int diff = expect - actual; \
        if (diff < 0) diff = -diff; \
        DEXPECT_EQ_BASE((diff <= miss), expect, actual, "%d"); \
    } while(0)


#define DEXPECT(condition) DCheckHelper(__FILE__, __LINE__, #condition, condition)

static inline DVoid DCheckHelper(DCStr file, DInt32 line, DCStr source, DBool condition)
{
    // 条件不满足就退出
    test_count++;
    if (!condition)
    {
        printf("%s:%d:\nEXPECT(%s) failed\n", file, line, source);
        if (assert_mode) {
            assert(0);
        }
    }
    else
    {
        test_pass++;
    }
}

#define DEXPECT_EQ(a, b) DCheckEqualsHelper(__FILE__, __LINE__, #a, a, #b, b)

// 利用 C++ 的重载，各种版本的比较(const char*, int, int64, double)
// const char* 版本
static inline DVoid DCheckEqualsHelper(DCStr file, DInt32 line,
    DCStr expected_source, DCStr expected,
    DCStr value_source, DCStr value)
{
    test_count++;

    if ((expected == NULL && value != NULL) || (expected != NULL && value == NULL))
    {
        assert(0);
    }

    // 直接用 strcmp 对比
    if ((expected != NULL && value != NULL && strcmp(expected, value) != 0))
    {
        printf("%s:%d:\nEXPECT_EQ(%s, %s) failed\n"
            "#  Expected: %s\n"
            "#  Found:    %s\n",
            file, line, expected_source, value_source, expected, value);
        if (assert_mode) {
            assert(0);
        }
    }
    else
    {
        test_pass++;
    }
}

// int 版本
static inline DVoid DCheckEqualsHelper(DCStr file, DInt32 line,
    DCStr expected_source, DInt32 expected,
    DCStr value_source, DInt32 value)
{
    test_count++;

    if (expected != value)
    {
        printf("%s:%d:\nEXPECT_EQ(%s, %s) failed\n"
            "#  Expected: %d\n"
            "#  Found:    %d\n",
            file, line, expected_source, value_source, expected, value);
        if (assert_mode) {
            assert(0);
        }
    }
    else
    {
        test_pass++;
    }
}

// int64 版本
static inline DVoid DCheckEqualsHelper(DCStr file, DInt32 line,
    DCStr expected_source, DInt64 expected,
    DCStr value_source, DInt64 value)
{
    test_count++;

    if (expected != value)
    {
        printf("%s:%d:\nEXPECT_EQ(%s, %s) failed\n"
            "#  Expected: %lld\n"
            "#  Found:    %lld\n",
            file, line, expected_source, value_source, expected, value);
        if (assert_mode) {
            assert(0);
        }
    }
    else
    {
        test_pass++;
    }
}

// double 版本
static inline DVoid DCheckEqualsHelper(DCStr file, DInt32 line,
    DCStr expected_source, DDouble expected,
    DCStr value_source, DDouble value)
{
    test_count++;

    // If expected and value are NaNs then expected != value.
    if (expected != value && (expected == expected || value == value))
    {
        printf("%s:%d:\nEXPECT_EQ(%s, %s) failed\n"
            "#  Expected: %.30e\n"
            "#  Found:    %.30e\n",
            file, line, expected_source, value_source, expected, value);
        if (assert_mode) {
            assert(0);
        }
    }
    else
    {
        test_pass++;
    }
}

//Output Result
#define TestResult(mod_name) printf("[%s] %d/%d (%3.2f%%) passed" D_LINES, mod_name, test_pass, test_count, (test_count!=0)? (double)(test_pass*100.0/test_count):0)
#define ShowResult() TestResult(__FUNCTION__)
