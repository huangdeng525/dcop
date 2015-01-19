/// -------------------------------------------------
/// test_iobject.h : 主要测试frameworks iobject操作
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TEST_IOBJECT_H_
#define _TEST_IOBJECT_H_

#include "test.h"
#include "Object_if.h"
#include "Manager_if.h"
#include "BaseMessage.h"
#include "BaseID.h"


enum TEST_IOBJECT_EVENT{
    TEST_IOBJECT_EVENT_TESTNO_SET = DCOP_MSG_CUSTOM,
    TEST_IOBJECT_EVENT_TESTNO_GET
};


enum TEST_OBJ_ID
{
    TEST_OBJ_ID_A = DCOP_OBJECT_CUSTOM,
    TEST_OBJ_ID_B,
};


/// 定义ITestForIObjectA版本号
INTF_VER(ITestForIObjectA, 1, 0, 0);


/// 定义ITestForIObjectA接口类
interface ITestForIObjectA : public IObject
{
    /// 设置接口
    virtual void SetTestNo(DWORD dwTestNo) = 0;

    /// 查询接口
    virtual DWORD GetTestNo() = 0;
};

/// 定义CTestForIObjectA实现类
class CTestForIObjectA : public ITestForIObjectA
{
public:
    CTestForIObjectA(Instance *piParent, int argc, char **argv);
    ~CTestForIObjectA();

    DCOP_DECLARE_INSTANCE;
    DCOP_DECLARE_IOBJECT;

    DWORD Init(IObject *root, int argc, void **argv);

    void SetTestNo(DWORD dwTestNo);
    DWORD GetTestNo();

private:
    IManager *m_piManager;
};

/// 定义CTestForIObjectB实现类
class CTestForIObjectB : public IObject
{
public:
    CTestForIObjectB(Instance *piParent, int argc, char **argv);
    ~CTestForIObjectB();

    DCOP_DECLARE_INSTANCE;
    DCOP_DECLARE_IOBJECT;

    DCOP_DECLARE_IOBJECT_MSG_HANDLE;

    void OnTestNoSet(objMsg *pMsg);
    void OnTestNoGet(objMsg *pMsg);

private:
    DWORD m_dwTestNo;
};

/// 测试frameworks IObject
class CTestSuite_IOBJECT : public ITestSuite
{
public:
    CTestSuite_IOBJECT();
    ~CTestSuite_IOBJECT();

    void BeforeTest();
    void AfterTest();

    int TestEntry(int argc, char* argv[]);

private:
    ITestForIObjectA *m_piTA;
    IObject *m_piTB;
    IManager *m_piManager;
};


#endif // #ifndef _TEST_IOBJECT_H_

