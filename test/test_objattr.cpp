/// -------------------------------------------------
/// test_objattr.cpp : 主要测试frameworks object attribute操作
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "test_objattr.h"
#include "Factory_if.h"
#include "Manager_if.h"


/// -------------------------------------------------
/// OBJATTR 测试用例
/// -------------------------------------------------
TEST_SUITE_TABLE(OBJATTR)
    TEST_SUITE_ITEM(CTestSuite_OBJATTR)
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
/// OBJATTR 测试套
/// -------------------------------------------------
IMPLEMENT_REGTESTSUITE_FUNC(OBJATTR)

/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CTestAttrForIObjectA, "TA_A")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CTestAttrForIObjectA)
    DCOP_IMPLEMENT_INTERFACE(ITestAttrForIObjectA)
    DCOP_IMPLEMENT_INTERFACE(IObject)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 实现对象类
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT(CTestAttrForIObjectA)
    DCOP_IMPLEMENT_IDENTIFY_STATIC("TA_A", TEST_OBJ_ID_A)
    DCOP_IMPLEMENT_CONFIG_THREADSAFE("threadsafe")
DCOP_IMPLEMENT_IOBJECT_END

/// -------------------------------------------------
/// 实现属性
/// -------------------------------------------------
IMPLEMENT_ATTRIBUTE(CTestAttrForIObjectA, dwTestNo, 1, CAttribute::TYPE_VAR)


CTestAttrForIObjectA::CTestAttrForIObjectA(Instance *piParent, int argc, char **argv) : 
    m_dwTestNo(100)
{
    DCOP_CONSTRUCT_INSTANCE(piParent);
    DCOP_CONSTRUCT_IOBJECT(argc, argv);

    m_piManager = 0;
}

CTestAttrForIObjectA::~CTestAttrForIObjectA()
{
    DCOP_RELEASE_INSTANCE(m_piManager);

    DCOP_DESTRUCT_IOBJECT();
    DCOP_DESTRUCT_INSTANCE();
}

DWORD CTestAttrForIObjectA::Init(IObject *root, int argc, void **argv)
{
    if (root->QueryInterface(ID_INTF(IManager), REF_PTR(m_piManager), this) != SUCCESS)
    {
        OSASSERT(0);
    }

    INIT_ATTRIBUTE_START(testIndex, 0, 0)
        INIT_ATTRIBUTE_MEMBER(dwTestNo)
    INIT_ATTRIBUTE_END

    return SUCCESS;
}

void CTestAttrForIObjectA::SetTestNo(DWORD dwTestNo)
{
    m_dwTestNo = dwTestNo;
}

DWORD CTestAttrForIObjectA::GetTestNo()
{
    return m_dwTestNo;
}


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CTestAttrForIObjectB, "TA_B")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CTestAttrForIObjectB)
    DCOP_IMPLEMENT_INTERFACE(IObject)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 实现对象类
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT(CTestAttrForIObjectB)
    DCOP_IMPLEMENT_IDENTIFY_STATIC("TA_B", TEST_OBJ_ID_B)
    DCOP_IMPLEMENT_CONFIG_THREADSAFE("threadsafe")
DCOP_IMPLEMENT_IOBJECT_END


CTestAttrForIObjectB::CTestAttrForIObjectB(Instance *piParent, int argc, char **argv)
{
    DCOP_CONSTRUCT_INSTANCE(piParent);
    DCOP_CONSTRUCT_IOBJECT(argc, argv);

    m_dwTestNo = 0;
}

CTestAttrForIObjectB::~CTestAttrForIObjectB()
{
    m_dwTestNo = 0;

    DCOP_DESTRUCT_IOBJECT();
    DCOP_DESTRUCT_INSTANCE();
}

void CTestAttrForIObjectB::OnTestNoSet(IObject *sender, DWORD event, void *para, DWORD paralen)
{
    if (para && (paralen >= sizeof(DWORD)))
    {
        m_dwTestNo = (*(DWORD *)para);
        TRACE_LOG(STR_FORMAT("OnTestNoSet: %d(addr:%p,len:%d)", m_dwTestNo, para, paralen));
    }
}

void CTestAttrForIObjectB::OnTestNoGet(IObject *sender, DWORD event, void *para, DWORD paralen)
{
    if (para && (paralen >= sizeof(DWORD)))
    {
        *(DWORD *)para = m_dwTestNo;
        TRACE_LOG(STR_FORMAT("OnTestNoGet: %d(addr:%p,len:%d)", m_dwTestNo, para, paralen));
    }
}

/// =============== 测试OBJATTR实现类 ===============
/// 测试说明: 
///     iTA有接口和属性的对象
///     iTB是一个无接口的对象
///     iTA是iTB的父对象
///     iTA属性触发的事件广播到iTB

CTestSuite_OBJATTR::CTestSuite_OBJATTR()
{
    m_piTA = 0;
    m_piTB = 0;
    m_piManager = 0;
}

CTestSuite_OBJATTR::~CTestSuite_OBJATTR()
{
    m_piTA = 0;
    m_piTB = 0;
    m_piManager = 0;
}

void CTestSuite_OBJATTR::BeforeTest()
{
    DCOP_CREATE_INSTANCE(IManager, "manager", NULL, 0, 0, m_piManager);
    OSASSERT(m_piManager != NULL);

    DCOP_CREATE_INSTANCE(ITestAttrForIObjectA, "TA_A", m_piManager, 0, 0, m_piTA);
    (void)m_piManager->InsertObject(m_piTA);

    DCOP_CREATE_INSTANCE(IObject, "TA_B", m_piTA, 0, 0, m_piTB);
    (void)m_piManager->InsertObject(m_piTB);

    (void)m_piManager->Init(NULL, 0, 0);
}

void CTestSuite_OBJATTR::AfterTest()
{
    if (m_piManager) m_piManager->Fini();

    DCOP_RELEASE_INSTANCE_REFER(0, m_piManager);
}

int CTestSuite_OBJATTR::TestEntry(int argc, char* argv[])
{
    if ((argc < 1) || (!argv))
    {
        return -1;
    }

    int iTestNo = atoi(argv[0]);

    if (!m_piTA || !m_piTB)
    {
        return -2;
    }

    m_piTA->SetTestNo((DWORD)iTestNo);
    if ((DWORD)iTestNo != m_piTA->GetTestNo())
    {
        return -3;
    }

    return 0;
}

