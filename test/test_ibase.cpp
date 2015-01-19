/// -------------------------------------------------
/// test_ibase.cpp : 主要测试frameworks ibase操作
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "test_ibase.h"
#include "Factory_if.h"


/// -------------------------------------------------
/// INSTANCE 测试用例
/// -------------------------------------------------
TEST_SUITE_TABLE(IBASE)
    TEST_SUITE_ITEM(CTestSuiteIBase)
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
/// INSTANCE 测试套
/// -------------------------------------------------
IMPLEMENT_REGTESTSUITE_FUNC(IBASE)

/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CTestForInstance, "TestForInstance")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CTestForInstance)
    DCOP_IMPLEMENT_INTERFACE(ITestForInstance)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END


CTestForInstance::CTestForInstance(Instance *piParent, int argc, char **argv)
{
    DCOP_CONSTRUCT_INSTANCE(piParent);

    m_dwTestNo = 0;
}

CTestForInstance::~CTestForInstance()
{
    m_dwTestNo = 0;

    DCOP_DESTRUCT_INSTANCE();
}

void CTestForInstance::SetTestNo(DWORD dwTestNo)
{
    m_dwTestNo = dwTestNo;
}

DWORD CTestForInstance::GetTestNo()
{
    return m_dwTestNo;
}


/// ================ 测试Instance实现类 ================

CTestSuiteIBase::CTestSuiteIBase()
{
    m_piBase = 0;
    m_piTestForInstance = 0;
}

CTestSuiteIBase::~CTestSuiteIBase()
{
    m_piBase = 0;
    m_piTestForInstance = 0;
}

void CTestSuiteIBase::BeforeTest()
{
    DCOP_CREATE_INSTANCE(Instance, "TestForInstance", 0, 0, 0, m_piBase);
}

void CTestSuiteIBase::AfterTest()
{
    DCOP_RELEASE_INSTANCE_REFER(0, m_piBase);
}

int CTestSuiteIBase::TestEntry(int argc, char* argv[])
{
    if ((argc < 1) || (!argv))
    {
        return -1;
    }

    int iTestNo = atoi(argv[0]);

    if (!m_piBase)
    {
        return -2;
    }

    if (m_piBase->QueryInterface(ID_INTF(ITestForInstance), REF_PTR(m_piTestForInstance)) != SUCCESS)
    {
        return -3;
    }

    m_piTestForInstance->SetTestNo((DWORD)iTestNo);
    if (m_piTestForInstance->GetTestNo() != (DWORD)iTestNo)
    {
        (void)m_piTestForInstance->Release();
        return -4;
    }

    (void)m_piTestForInstance->Release();
    return 0;
}

