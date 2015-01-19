/// -------------------------------------------------
/// test_argcfg.h : 主要测试参数配置操作
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TEST_ARGCFG_H_
#define _TEST_ARGCFG_H_

#include "test.h"
#include "cfg/argcfg.h"


/// 测试argcfg
class CTestSuite_ARGCFG : public ITestSuite
{
public:
    CTestSuite_ARGCFG();
    ~CTestSuite_ARGCFG();

    static void Print(const char *);

    int TestEntry(int argc, char* argv[]);

private:
    CArgCfgTable m_cfgTable;
    DECLARE_CONFIG_ITEM(BYTE, a);
    DECLARE_CONFIG_ITEM(WORD, b);
    DECLARE_CONFIG_ITEM(WORD, c);
    DECLARE_CONFIG_ITEM(DWORD, d);
    DECLARE_CONFIG_ITEM(DWORD, e);
    DECLARE_CONFIG_ITEM(char *, f);
};


#endif // #ifndef _TEST_ARGCFG_H_

