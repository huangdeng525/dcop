/// -------------------------------------------------
/// test_algo.cpp : ÷˜“™≤‚ ‘À„∑®≤Ÿ◊˜
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "test_algo.h"
#include "algo/search.h"
#include "algo/BaseN.h"


/// -------------------------------------------------
/// ≤‚ ‘”√¿˝
/// -------------------------------------------------
TEST_SUITE_TABLE(ALGO)
    TEST_SUITE_ITEM(CTestSuiteAlgo)
        TEST_CASE_ITEM(1)
            "1"
        TEST_CASE_ITEM_END
    TEST_SUITE_ITEM_END
TEST_SUITE_TABLE_END

/// -------------------------------------------------
/// ≤‚ ‘Ã◊
/// -------------------------------------------------
IMPLEMENT_REGTESTSUITE_FUNC(ALGO)


CTestSuiteAlgo::CTestSuiteAlgo()
{
}

CTestSuiteAlgo::~CTestSuiteAlgo()
{
}

int CTestSuiteAlgo::TestEntry(int argc, char* argv[])
{
    int rc = BinarySearchEntry();
    rc |= BaseNEntry();

    return rc;
}

int CTestSuiteAlgo::BinarySearchEntry()
{
    int array[] = {0, 1, 2, 3, 4, 5, 6, 7, 13, 19};
    int rc = 0;

    int t = 13;
    int m = binary_search(array, sizeof(array)/sizeof(array[0]), t);
    printf("binary_search : %d\r\n", m);
    TEST_CHECK(t, array[m], rc);

    t = 9;
    m = binary_search(array, sizeof(array)/sizeof(array[0]), t);
    printf("binary_search : %d\r\n", m);
    TEST_CHECK(-1, m, rc);

    t = 19;
    m = binary_search_recurse(array, 0, sizeof(array)/sizeof(array[0]), t);
    printf("binary_search_recurse : %d\r\n", m);
    TEST_CHECK(t, array[m], rc);

    t = 10;
    m = binary_search_recurse(array, 0, sizeof(array)/sizeof(array[0]), t);
    printf("binary_search_recurse : %d\r\n", m);
    TEST_CHECK(-1, m, rc);

    return rc;
}

int CTestSuiteAlgo::BaseNCase(unsigned int N, unsigned int num)
{
    unsigned int array[8];
    unsigned int i = 0;
    unsigned int high = 0;
    int rc = 0;

    rc |= DCOP_BaseN(N, num, array, ARRAY_SIZE(array), &high);
    printf(" <rc:%d> <high:%d> Base%d(%d) = ", rc, high, N, num);
    for (i = 0; i < ARRAY_SIZE(array); ++i)
    {
        printf("%d ", array[ARRAY_SIZE(array) - i - 1]);
    }
    printf("\r\n");

    unsigned int out = 0;
    rc |= DCOP_Base10(N, array, ARRAY_SIZE(array), &out);
    printf(" <rc:%d> Base10() = %d \r\n", rc, out);

    TEST_CHECK(num, out, rc);

    return rc;
}

int CTestSuiteAlgo::BaseNEntry()
{
    int rc = BaseNCase(16, 1376306);
    rc |= BaseNCase(62, 138);

    return rc;
}

