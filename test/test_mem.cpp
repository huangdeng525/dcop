/// -------------------------------------------------
/// test_mem.cpp : Ö÷Òª²âÊÔÄÚ´æ²Ù×÷
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "test_mem.h"


/// -------------------------------------------------
/// ²âÊÔÓÃÀý
/// -------------------------------------------------
TEST_SUITE_TABLE(MEM)
    TEST_SUITE_ITEM(CTestSuiteMem)
        TEST_CASE_ITEM(1)
            "1"
        TEST_CASE_ITEM_END
        TEST_CASE_ITEM(1)
            "10"
        TEST_CASE_ITEM_END
        TEST_CASE_ITEM(1)
            "100"
        TEST_CASE_ITEM_END
        TEST_CASE_ITEM(1)
            "1000"
        TEST_CASE_ITEM_END
    TEST_SUITE_ITEM_END
TEST_SUITE_TABLE_END

/// -------------------------------------------------
/// ²âÊÔÌ×
/// -------------------------------------------------
IMPLEMENT_REGTESTSUITE_FUNC(MEM)


CTestSuiteMem::CTestSuiteMem()
{
}

CTestSuiteMem::~CTestSuiteMem()
{
}

int CTestSuiteMem::TestEntry(int argc, char* argv[])
{
    if ((argc < 1) || (!argv))
    {
        return -1;
    }

    int iAllocCount = atoi(argv[0]);

    for (int i = 0; i < iAllocCount; ++i)
    {
        CTestMem *pTestMem1 = new CTestMem;
        pTestMem1->SetTest(iAllocCount);
        iAllocCount = pTestMem1->GetTest();
        delete pTestMem1;
        pTestMem1 = 0;
    }

    if (iAllocCount != atoi(argv[0]))
    {
        return -2;
    }

    CTestMem *pTestMem2 = (CTestMem *)malloc(sizeof(CTestMem));
    pTestMem2->SetTest(iAllocCount);
    iAllocCount = pTestMem2->GetTest();
    free(pTestMem2);
    pTestMem2 = 0;

    if (iAllocCount != atoi(argv[0]))
    {
        return -3;
    }

    return 0;
}

