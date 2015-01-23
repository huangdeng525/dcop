/// -------------------------------------------------
/// test_random.cpp : 主要测试随机数操作
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "test_random.h"
#include "task.h"


/// -------------------------------------------------
/// 测试用例
/// -------------------------------------------------
TEST_SUITE_TABLE(RANDOM)
    TEST_SUITE_ITEM(CTestSuiteRandom)
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
/// 测试套
/// -------------------------------------------------
IMPLEMENT_REGTESTSUITE_FUNC(RANDOM)


CTestSuiteRandom::CTestSuiteRandom() : m_aRandomList(RAND_SIZE)
{
    RecordMemDetail(1, 1, "darray");
}

CTestSuiteRandom::~CTestSuiteRandom()
{
}

int CTestSuiteRandom::TestEntry(int argc, char* argv[])
{
    int rc = 0;

    DWORD dwCount = 10;
    if (argc && argv && (*argv))
    {
        dwCount = (DWORD)atoi(argv[0]);
    }

    objRandom *pRandom = DCOP_CreateRandom();
    if (!pRandom)
    {
        return (-1);
    }

    for (DWORD i = 0; i < dwCount; ++i)
    {
        char buf[RAND_SIZE];
        (void)memset(buf, 0, sizeof(buf));
        pRandom->Gen(buf, sizeof(buf));
        if (dwCount <= 10)
        {
            PrintBuffer(STR_FORMAT("Random %d", (i+1)), buf, sizeof(buf), PrintToConsole, 0);
        }
        TEST_CHECK(SUCCESS, Append(buf), rc);
    }

    delete pRandom;
    m_aRandomList.Clear();

    return rc;
}

DWORD CTestSuiteRandom::Append(void *pBuf)
{
    if (m_aRandomList.Find(pBuf) < m_aRandomList.Count())
    {
        return FAILURE;
    }

    int rc = 0;
    TEST_CHECK(SUCCESS, m_aRandomList.Append(pBuf), rc);
    return (!rc)? SUCCESS : FAILURE;
}

