/// -------------------------------------------------
/// test_argcfg.cpp : 主要测试参数配置操作
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "test_argcfg.h"


/// -------------------------------------------------
/// IOBJECT 测试用例
/// -------------------------------------------------
TEST_SUITE_TABLE(ARGCFG)
    TEST_SUITE_ITEM(CTestSuite_ARGCFG)
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
        TEST_CASE_ITEM(1)
            "4294967295"
        TEST_CASE_ITEM_END
    TEST_SUITE_ITEM_END
TEST_SUITE_TABLE_END

/// -------------------------------------------------
/// IOBJECT 测试套
/// -------------------------------------------------
IMPLEMENT_REGTESTSUITE_FUNC(ARGCFG)

/// -------------------------------------------------
/// 实现配置项
/// -------------------------------------------------
IMPLEMENT_CONFIG_ITEM(CTestSuite_ARGCFG, a, "-a", "--asdf-gh", "1asdfghjkl", CArgCfgType::TYPE_VALUE, true)
IMPLEMENT_CONFIG_ITEM(CTestSuite_ARGCFG, b, "-b", "--bnmz-xc", "2asdfghjkl", CArgCfgType::TYPE_SWITCH, true)
IMPLEMENT_CONFIG_ITEM(CTestSuite_ARGCFG, c, "-c", "--csdf-gh", "3asdfghjkl", CArgCfgType::TYPE_SWITCH, true)
IMPLEMENT_CONFIG_ITEM(CTestSuite_ARGCFG, d, "-d", "--dsdf-gh", "4asdfghjkl", CArgCfgType::TYPE_VALUE, false)
IMPLEMENT_CONFIG_ITEM(CTestSuite_ARGCFG, e, "-e", "--esdf-gh", "5asdfghjkl", CArgCfgType::TYPE_VALUE, false)
IMPLEMENT_CONFIG_ITEM(CTestSuite_ARGCFG, f, "-f", "--fsdf-gh", "6asdfghjkl", CArgCfgType::TYPE_STRING, false)


CTestSuite_ARGCFG::CTestSuite_ARGCFG()
{
    INIT_CONFIG_START(m_cfgTable)
        INIT_CONFIG_ITEM_VAL(a, 100)
        INIT_CONFIG_ITEM_VAL(b, 10000)
        INIT_CONFIG_ITEM_VAL(c, 11200)
        INIT_CONFIG_ITEM_VAL(d, 603210)
        INIT_CONFIG_ITEM_VAL(e, 0xffffffff)
        INIT_CONFIG_ITEM_STR(f, "Test", 32)
    INIT_CONFIG_END
}

CTestSuite_ARGCFG::~CTestSuite_ARGCFG()
{
}

void CTestSuite_ARGCFG::Print(const char *cszStr)
{
    if (cszStr)
    {
        printf("%s\n", cszStr);
    }
}

int CTestSuite_ARGCFG::TestEntry(int argc, char* argv[])
{
    m_cfgTable.SetPrintFunc((LOG_PRINT)Print, 0);

    m_cfgTable.Cfg(argc, argv, false);

    return 0;
}



