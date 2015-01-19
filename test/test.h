/// -------------------------------------------------
/// test.cpp : 测试入口头入口头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TEST_TEST_H_
#define _TEST_TEST_H_

/// [gtest的头文件一定要放最前面，否则会报错]
#include <gtest/gtest.h>
#include "dcop.h"
#include "list/tailq.h"


/// -------------------------------------------------
/// 单个测试用例中的最多参数个数
/// -------------------------------------------------
#define TEST_CASE_ARG_COUNT_MAX 30

/// -------------------------------------------------
/// 单个测试套中的最大用例数
/// -------------------------------------------------
#define TEST_SUITE_CASE_COUNT_MAX 30


/// -------------------------------------------------
/// 测试用例类
/// -------------------------------------------------
class CTestCase
{
public:
    int argc;           // 参数个数
    char** argv;        // 参数列表(会指向本对象后面的空间)

public:
    TAILQ_ENTRY(CTestCase) m_field;         // 指向下一个测试用例
};


/// -------------------------------------------------
/// 测试套基类
/// -------------------------------------------------
class ITestSuite
{
public:
    ITestSuite();
    virtual ~ITestSuite();

    /// 测试套开始前的准备工作
    virtual void BeforeTest();

    /// 测试入口(每个测试用例会调用一次;返回0为成功,非0为失败)
    virtual int TestEntry(int argc, char* argv[]);

    /// 测试套结束后的清理工作
    virtual void AfterTest();

    /// 注册测试套
    static void RegTestSuite(ITestSuite *pTest, const char *title = 0);

    /// 设置和获取测试套标题
    void SetTestTitle(const char *title) { m_testTitle = title; }
    const char *GetTestTitle() { return m_testTitle; }

    /// 测试结果
    void SetTestResult(int rc) { (!rc)? m_testSuccessCount++ : m_testFailureCount++; }
    int GetSuccessCount() { return m_testSuccessCount; }
    int GetFailureCount() { return m_testFailureCount; }

public:
    TAILQ_ENTRY(ITestSuite) m_field;            // 指向下一个测试套
    TAILQ_HEAD(CTestCase) m_testCases;          // 所属的测试用例集合
    const char *m_testTitle;                    // 测试套标题
    int m_testSuccessCount;                     // 测试成功的用例数量
    int m_testFailureCount;                     // 测试失败的用例数量
};


/// -------------------------------------------------
/// 测试套注册结构
/// -------------------------------------------------
typedef struct tagTestSuiteRegNode
{
    const char *testTitle;
    ITestSuite *testSuite;
    struct
    {
        int argc;
        char *argv[TEST_CASE_ARG_COUNT_MAX];
    } testCases[TEST_SUITE_CASE_COUNT_MAX];
}TestSuiteRegNode;


/// -------------------------------------------------
/// 测试套注册表头
/// -------------------------------------------------
#define TEST_SUITE_TABLE(TestName)                  \
    TestSuiteRegNode g_aReg##TestName##TestSuiteTable[] = {


/// -------------------------------------------------
/// 测试套注册表尾
/// -------------------------------------------------
#define TEST_SUITE_TABLE_END                        \
    };


/// -------------------------------------------------
/// 测试套注册表项
/// -------------------------------------------------
#define TEST_SUITE_ITEM(CTestSuite)                 \
    {#CTestSuite, new CTestSuite, {


/// -------------------------------------------------
/// 测试套中的测试用例列表表项
/// -------------------------------------------------
#define TEST_CASE_ITEM(num)                         \
    {num, {(char *)


/// -------------------------------------------------
/// 测试套中的测试用例列表表项结束
/// -------------------------------------------------
#define TEST_CASE_ITEM_END                          \
    }},


/// -------------------------------------------------
/// 测试套注册表项结束
/// -------------------------------------------------
#define TEST_SUITE_ITEM_END                         \
    {0}}},


/// -------------------------------------------------
/// 检查测试结果
/// -------------------------------------------------
#define TEST_CHECK(expected, actual, rc)            \
do {                                                \
    int expValue = 0;                               \
    int actValue = 0;                               \
    EXPECT_EQ((expValue = (int)(expected)), (actValue = (int)(actual))); \
    if (expValue != actValue) rc |= (-1);           \
} while (0)


/// -------------------------------------------------
/// 单次加载对象
/// -------------------------------------------------
#define TEST_LOAD_OBJECT(Manager, Intf, Name, ID, Obj) \
    char *Name##_name = #Name;                      \
    char Name##_id[16];                             \
    (void)snprintf(Name##_id, sizeof(Name##_id), "%d", ID); \
    char *Name##_arg[] = {"-name", Name##_name, "-id", Name##_id}; \
    DCOP_CREATE_INSTANCE(Intf, #Name, Manager, ARRAY_SIZE(Name##_arg), Name##_arg, Obj); \
    (void)Manager->InsertObject(Obj)


/// -------------------------------------------------
/// 实现测试套注册函数
/// -------------------------------------------------
#define IMPLEMENT_REGTESTSUITE_FUNC(TestName)       \
    void vReg##TestName##TestSuiteFunc()            \
{                                                   \
    RegTestSuites(g_aReg##TestName##TestSuiteTable, ARRAY_SIZE(g_aReg##TestName##TestSuiteTable)); \
}


/// -------------------------------------------------
/// 调用测试套注册函数
/// -------------------------------------------------
#define CALL_REGTESTSUITE_FUNC(ts)                  \
    extern void vReg##ts##TestSuiteFunc();          \
    vReg##ts##TestSuiteFunc()


/// -------------------------------------------------
/// 测试套注册函数
/// -------------------------------------------------
extern void RegTestSuites(TestSuiteRegNode *pRegTable, int iRegCount);


/// -------------------------------------------------
/// 外部调用的测试总入口
/// -------------------------------------------------
extern int test_entry(int argc, char* argv[]);


#endif // #ifndef _TEST_TEST_H_

