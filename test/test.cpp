/// -------------------------------------------------
/// test.cpp : 测试入口实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "test.h"
#include <assert.h>

#if DCOP_OS == DCOP_OS_WINDOWS
#include "console/color/ConsoleColor.h"
#pragma comment(lib, "gtestd")
#endif


/// -------------------------------------------------
/// 强制连接.a库中的一些原本自动执行的全局构建符号
/// -------------------------------------------------
CPPBUILDUNIT_FORCELINK(vRegOsTaskStubFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegOsSemStubFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCObjectManagerToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCDispatchToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCNotifyToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCControlToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCTimerToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCScheduleToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCStatusToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCResponseToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCModelToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCDataToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCDataMemToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCDataFileToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCDataMySQLToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCTlvToSQLToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCConnectToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCProxyToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCSessionToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCUserToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCSecureToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCAccessToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCCommandToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCHttpServerToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCHttpRequestToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCHttpProcessToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCHttpJsonToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCHttpResponseToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCMonitorToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCAppBaseToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCPythonXToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCLuaXToFactoryFunc, 0);
CPPBUILDUNIT_FORCELINK(vRegCTestForIObjectAToFactoryFunc, 0);


/// -------------------------------------------------
/// 测试套管理器初始化
/// -------------------------------------------------
TAILQ_HEAD(ITestSuite) g_testSuites;            // 全局测试套队列
int g_testCasesCount;                           // 全局测试用例数量
int g_testSuccessCount;                         // 全局测试成功的用例数量
int g_testFailureCount;                         // 全局测试失败的用例数量
void vInitTestSuitesFunc()
{
    /// 初始化测试套管理器
    TAILQ_INIT(&g_testSuites);
    g_testCasesCount = 0;
    g_testSuccessCount = 0;
    g_testFailureCount = 0;

    /// 这里添加新的测试套
#if 0
    CALL_REGTESTSUITE_FUNC(TASK);
#endif
    CALL_REGTESTSUITE_FUNC(GRAPH);
    CALL_REGTESTSUITE_FUNC(MEM);
    CALL_REGTESTSUITE_FUNC(XML);
    CALL_REGTESTSUITE_FUNC(AVL);
    CALL_REGTESTSUITE_FUNC(IBASE);
    CALL_REGTESTSUITE_FUNC(IOBJECT);
    CALL_REGTESTSUITE_FUNC(OBJATTR);
    CALL_REGTESTSUITE_FUNC(TIMER);
    CALL_REGTESTSUITE_FUNC(DATA);
    CALL_REGTESTSUITE_FUNC(STREAM);
    CALL_REGTESTSUITE_FUNC(MD5);
    CALL_REGTESTSUITE_FUNC(ALGO);
    CALL_REGTESTSUITE_FUNC(RANDOM);

}
/// 这里自动添加到管理器
/// CPPBUILDUNIT_AUTO(vInitTestSuitesFunc, 0);


/// -------------------------------------------------
/// 测试套管理器构造函数
/// -------------------------------------------------
ITestSuite::ITestSuite()
{
    TAILQ_NEXT(this, m_field) = 0;
    TAILQ_INIT(&m_testCases);

    m_testTitle = "";
    m_testSuccessCount = 0;
    m_testFailureCount = 0;
}

/// -------------------------------------------------
/// 测试套管理器析构函数
/// -------------------------------------------------
ITestSuite::~ITestSuite()
{
    TAILQ_NEXT(this, m_field) = 0;
    CTestCase *pTestCase = TAILQ_FIRST(&m_testCases);
    while (pTestCase)
    {
        CTestCase *pTmp = pTestCase;
        pTestCase = TAILQ_NEXT(pTestCase, m_field);

        free((void *)pTmp);
        pTmp = 0;
    }
}

/// -------------------------------------------------
/// 测试套管理器默认实现测试前
/// -------------------------------------------------
void ITestSuite::BeforeTest()
{
}

/// -------------------------------------------------
/// 测试套管理器默认实现测试入口
/// -------------------------------------------------
int ITestSuite::TestEntry(int argc, char* argv[])
{
    return -1;
}

/// -------------------------------------------------
/// 测试套管理器默认实现测试后
/// -------------------------------------------------
void ITestSuite::AfterTest()
{
}

/// -------------------------------------------------
/// 测试套管理器注册一个测试套
/// -------------------------------------------------
void ITestSuite::RegTestSuite(ITestSuite *pTestSuite, const char *title)
{
    assert(pTestSuite);
    TAILQ_PUSH_BACK(&g_testSuites, pTestSuite, m_field);
    pTestSuite->SetTestTitle(title);
}

/// -------------------------------------------------
/// 全局清除所有测试套
/// -------------------------------------------------
void ClrTestSuites()
{
    ITestSuite *pTestSuite = TAILQ_FIRST(&g_testSuites);
    while (pTestSuite)
    {
        ITestSuite *pTmp = pTestSuite;
        pTestSuite = TAILQ_NEXT(pTestSuite, m_field);

        delete pTmp;
        pTmp = 0;
    }
}

/// -------------------------------------------------
/// 全局注册一个测试套
/// -------------------------------------------------
void RegTestSuites(TestSuiteRegNode *pRegTable, int iRegCount)
{
    for (int i = 0; i < iRegCount; ++i)
    {
        ITestSuite::RegTestSuite(pRegTable[i].testSuite, pRegTable[i].testTitle);
        int j = 0;
        while (pRegTable[i].testCases[j].argc)
        {
            CTestCase *pTestCaseObj = (CTestCase *)malloc(sizeof(CTestCase) + 
                            pRegTable[i].testCases[j].argc * sizeof(char *));
            pTestCaseObj->argc = pRegTable[i].testCases[j].argc;
            pTestCaseObj->argv = (char **)(pTestCaseObj + 1);
            TAILQ_NEXT(pTestCaseObj, m_field) = 0;
            (void)memcpy((void *)(pTestCaseObj->argv), 
                         (void *)(pRegTable[i].testCases[j].argv), 
                         pTestCaseObj->argc * sizeof(char *));
            TAILQ_PUSH_BACK(&(pRegTable[i].testSuite->m_testCases), pTestCaseObj, m_field);
            j++;
            g_testCasesCount++;
        }
    }
}

/// -------------------------------------------------
/// 打印测试所有参数
/// -------------------------------------------------
void PrintTestArgs(int TestCaseNo, int argc, char* argv[], int bStartEnd, int rc = 0, DWORD cost = 0)
{
    printf("  [TC %d] %s", TestCaseNo++, (bStartEnd)? "Start(" : "End");
    if (bStartEnd) for (int i = 0; i < argc; ++i) { printf( (((i + 1) != argc)? "%s," : "%s) ... \n"), argv[i]); }
    else printf("(rc:%d)<costtime:%dms>!\n", rc, cost);
}

/// -------------------------------------------------
/// 调用所有测试用例
/// -------------------------------------------------
void CallTestCases(ITestSuite *pTestSuite)
{
    int rc = 0;
    int i = 0;

    if (!pTestSuite)
    {
        return;
    }

    CTestCase *pTestCase = TAILQ_FIRST(&(pTestSuite->m_testCases));
    if (!pTestCase)
    {
        /// 测试用例为空, 直接调用测试入口
        PrintTestArgs(0, 0, 0, true);
        rc = pTestSuite->TestEntry(0, 0);
        PrintTestArgs(0, 0, 0, false, rc);

        EXPECT_EQ(0, rc);

        /// 录入测试结果
        pTestSuite->SetTestResult(rc);
        (void)((!rc)? g_testSuccessCount++ : g_testFailureCount++);

        return;
    }

    /// 循环遍历所有测试用例
    while (pTestCase)
    {
        PrintTestArgs(++i, pTestCase->argc, pTestCase->argv, true);
        DCOP_START_TIME();
        rc = pTestSuite->TestEntry(pTestCase->argc, pTestCase->argv);
        DCOP_END_TIME();
        PrintTestArgs(i, pTestCase->argc, pTestCase->argv, false, rc, DCOP_COST_TIME());

        EXPECT_EQ(0, rc);

        pTestSuite->SetTestResult(rc);
        (void)((!rc)? g_testSuccessCount++ : g_testFailureCount++);

        pTestCase = TAILQ_NEXT(pTestCase, m_field);
    }

}

/// -------------------------------------------------
/// 外部调用的测试总入口
/// -------------------------------------------------
#include <iostream>
int main(int argc, char* argv[])
{
    std::cout << "Test Start ... " << std::endl;
    testing::InitGoogleTest(&argc, argv);
    int rc = RUN_ALL_TESTS();
    std::cout << "Test End! Press <Enter> To Quit ... " << std::endl;
    std::cin.get();
    return rc;
}

/// -------------------------------------------------
/// gtest调用的测试入口
/// -------------------------------------------------
TEST(test_main, TestBaseAssert)
{
    /// 打开日志记录
    DebugLogStatus(1);

    /// 打开内存跟踪
    DebugMemStatus(1);
    RecordAllocCallstack(1);
    OutputMemLog(1);

    /// 初始化测试套
    vInitTestSuitesFunc();

    /// 开始测试
    int i = 0;
    ITestSuite *pTestSuite = TAILQ_FIRST(&g_testSuites);
    while (pTestSuite)
    {
        printf("\n");
        printf("----------------------------------------------------------------------\n");
        printf("[TS %d '%s'] Start ... ... \n", ++i, pTestSuite->GetTestTitle());
        printf("----------------------------------------------------------------------\n");
        pTestSuite->BeforeTest();

        CallTestCases(pTestSuite);

        pTestSuite->AfterTest();
        printf("----------------------------------------------------------------------\n");
        printf("[TS %d '%s'] End(testcase:%d, success:%d, failure:%d)! \n", i, 
            pTestSuite->GetTestTitle(), TAILQ_COUNT(&(pTestSuite->m_testCases)), 
            pTestSuite->GetSuccessCount(), pTestSuite->GetFailureCount());
        printf("----------------------------------------------------------------------\n");
        printf("\n");

        pTestSuite = TAILQ_NEXT(pTestSuite, m_field);
    }

    printf("######################################################################\n");
    if (!g_testFailureCount)
        std::cout 
                #if DCOP_OS == DCOP_OS_WINDOWS
                    << green 
                #endif
                            << "    [TEST CASE REPORT]    TOTAL: " << g_testCasesCount
                            << "    SUCCESS: " << g_testSuccessCount
                            << "    FAILURE: " << g_testFailureCount
                #if DCOP_OS == DCOP_OS_WINDOWS
                    << white 
                #endif
                            << std::endl;
    else
        std::cout 
                #if DCOP_OS == DCOP_OS_WINDOWS
                    << red 
                #endif
                            << "    [TEST CASE REPORT]    TOTAL: " << g_testCasesCount
                            << "    SUCCESS: " << g_testSuccessCount
                            << "    FAILURE: " << g_testFailureCount
                #if DCOP_OS == DCOP_OS_WINDOWS
                    << white 
                #endif
                            << std::endl;
    printf("######################################################################\n");

    /// 测试结束
    ClrTestSuites();

    /// 关闭内存跟踪
    /// DebugMemStatus(0);

    /// 关闭日志记录
    /// DebugLogStatus(0);
}

