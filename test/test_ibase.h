/// -------------------------------------------------
/// test_ibase.h : 主要测试frameworks ibase操作
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TEST_INSTANCE_H_
#define _TEST_INSTANCE_H_

#include "test.h"
#include "BaseClass.h"


/// 定义ITestForInstance版本号
INTF_VER(ITestForInstance, 1, 0, 0);


/// 定义ITestForInstance接口类
interface ITestForInstance : public Instance
{
    /// 设置接口
    virtual void SetTestNo(DWORD dwTestNo) = 0;

    /// 查询接口
    virtual DWORD GetTestNo() = 0;
};


/// 定义CTestForInstance实现类
class CTestForInstance : public ITestForInstance
{
public:
    CTestForInstance(Instance *piParent, int argc, char **argv);
    ~CTestForInstance();

    DCOP_DECLARE_INSTANCE;

    void SetTestNo(DWORD dwTestNo);

    DWORD GetTestNo();

private:
    DWORD m_dwTestNo;
};


/// 测试frameworks Instance Base
class CTestSuiteIBase : public ITestSuite
{
public:
    CTestSuiteIBase();
    ~CTestSuiteIBase();

    void BeforeTest();
    void AfterTest();

    int TestEntry(int argc, char* argv[]);

private:
    Instance *m_piBase;
    ITestForInstance *m_piTestForInstance;
};


#endif // #ifndef _TEST_INSTANCE_H_

