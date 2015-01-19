/// -------------------------------------------------
/// test_iobject.h : 主要测试frameworks iobject操作
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "test_iobject.h"
#include "Factory_if.h"
#include "Manager_if.h"
#include "cpu/bytes.h"


/// -------------------------------------------------
/// IOBJECT 测试用例
/// -------------------------------------------------
TEST_SUITE_TABLE(IOBJECT)
    TEST_SUITE_ITEM(CTestSuite_IOBJECT)
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
IMPLEMENT_REGTESTSUITE_FUNC(IOBJECT)

/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CTestForIObjectA, "TA")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CTestForIObjectA)
    DCOP_IMPLEMENT_INTERFACE(ITestForIObjectA)
    DCOP_IMPLEMENT_INTERFACE(IObject)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 实现对象类
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT(CTestForIObjectA)
    DCOP_IMPLEMENT_IDENTIFY_STATIC("TA", TEST_OBJ_ID_A)
    DCOP_IMPLEMENT_CONFIG_THREADSAFE("threadsafe")
DCOP_IMPLEMENT_IOBJECT_END


CTestForIObjectA::CTestForIObjectA(Instance *piParent, int argc, char **argv)
{
    DCOP_CONSTRUCT_INSTANCE(piParent);
    DCOP_CONSTRUCT_IOBJECT(argc, argv);

    m_piManager = 0;
}

CTestForIObjectA::~CTestForIObjectA()
{
    DCOP_RELEASE_INSTANCE(m_piManager);

    DCOP_DESTRUCT_IOBJECT();
    DCOP_DESTRUCT_INSTANCE();
}

DWORD CTestForIObjectA::Init(IObject *root, int argc, void **argv)
{
    if (root->QueryInterface(ID_INTF(IManager), REF_PTR(m_piManager), this) != SUCCESS)
    {
        OSASSERT(0);
    }

    return SUCCESS;
}

void CTestForIObjectA::SetTestNo(DWORD dwTestNo)
{
    OSASSERT(m_piManager != NULL);

    objMsg *pMsg = DCOP_CreateMsg(sizeof(DWORD), TEST_IOBJECT_EVENT_TESTNO_SET, ID());
    if (!pMsg)
    {
        return;
    }

    BYTE *pbyData = (BYTE *)pMsg->GetDataBuf();
    OSASSERT(pbyData != NULL);

    Bytes_SetDword(pbyData, dwTestNo);

    m_piManager->BroadcastMsg(pMsg);

    delete pMsg;
}

DWORD CTestForIObjectA::GetTestNo()
{
    objMsg *pMsg = DCOP_CreateMsg(sizeof(DWORD), TEST_IOBJECT_EVENT_TESTNO_GET, ID());
    if (!pMsg)
    {
        return 0;
    }

    m_piManager->BroadcastMsg(pMsg);

    BYTE *pbyData = (BYTE *)pMsg->GetDataBuf();
    OSASSERT(pbyData != NULL);

    DWORD dwTestNo = Bytes_GetDword(pbyData);
    delete pMsg;

    return dwTestNo;
}


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CTestForIObjectB, "TB")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CTestForIObjectB)
    DCOP_IMPLEMENT_INTERFACE(IObject)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 实现对象类
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT(CTestForIObjectB)
    DCOP_IMPLEMENT_IDENTIFY_DYNAMIC("name", "id")
    DCOP_IMPLEMENT_CONFIG_THREADSAFE("threadsafe")
DCOP_IMPLEMENT_IOBJECT_END

/// -------------------------------------------------
/// 实现消息分发
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT_MSG_HANDLE(CTestForIObjectB)
    DCOP_IMPLEMENT_IOBJECT_MSG_PROC(TEST_IOBJECT_EVENT_TESTNO_SET, OnTestNoSet)
    DCOP_IMPLEMENT_IOBJECT_MSG_PROC(TEST_IOBJECT_EVENT_TESTNO_GET, OnTestNoGet)
DCOP_IMPLEMENT_IOBJECT_MSG_HANDLE_END


CTestForIObjectB::CTestForIObjectB(Instance *piParent, int argc, char **argv)
{
    DCOP_CONSTRUCT_INSTANCE(piParent);
    DCOP_CONSTRUCT_IOBJECT(argc, argv);

    m_dwTestNo = 0;
}

CTestForIObjectB::~CTestForIObjectB()
{
    m_dwTestNo = 0;

    DCOP_DESTRUCT_IOBJECT();
    DCOP_DESTRUCT_INSTANCE();
}

void CTestForIObjectB::OnTestNoSet(objMsg *pMsg)
{
    BYTE *pbyData = (BYTE *)pMsg->GetDataBuf();
    DWORD dwDataLen = pMsg->GetDataLen();

    if (pbyData && (dwDataLen >= sizeof(DWORD)))
    {
        m_dwTestNo = (*(DWORD *)pbyData);
    }
}

void CTestForIObjectB::OnTestNoGet(objMsg *pMsg)
{
    BYTE *pbyData = (BYTE *)pMsg->GetDataBuf();
    DWORD dwDataLen = pMsg->GetDataLen();

    if (pbyData && (dwDataLen >= sizeof(DWORD)))
    {
        *(DWORD *)pbyData = m_dwTestNo;
    }
}

/// =============== 测试IObject实现类 ===============
/// 测试说明: 
///     iTA是一个有接口的对象
///     iTB是一个无接口的对象
///     iTA是iTB的父对象
///     iTA会广播事件到iTB

CTestSuite_IOBJECT::CTestSuite_IOBJECT()
{
    m_piTA = 0;
    m_piTB = 0;
    m_piManager = 0;
}

CTestSuite_IOBJECT::~CTestSuite_IOBJECT()
{
    m_piTA = 0;
    m_piTB = 0;
    m_piManager = 0;
}

void CTestSuite_IOBJECT::BeforeTest()
{
    DCOP_CREATE_INSTANCE(IManager, "manager", NULL, 0, 0, m_piManager);
    OSASSERT(m_piManager != NULL);

    DCOP_CREATE_INSTANCE(ITestForIObjectA, "TA", m_piManager, 0, 0, m_piTA);
    (void)m_piManager->InsertObject(m_piTA);

    char *Name2 = "TB";
    char ID2[16];
    (void)snprintf(ID2, sizeof(ID2), "%d", TEST_OBJ_ID_B);
    char *arg2[] = {"-name", Name2, "-id", ID2};
    DCOP_CREATE_INSTANCE(IObject, Name2, m_piTA, ARRAY_SIZE(arg2), arg2, m_piTB);
    (void)m_piManager->InsertObject(m_piTB);

    (void)m_piManager->Init(NULL, 0, 0);
}

void CTestSuite_IOBJECT::AfterTest()
{
    if (m_piManager) m_piManager->Fini();

    DCOP_RELEASE_INSTANCE_REFER(0, m_piManager);
}

int CTestSuite_IOBJECT::TestEntry(int argc, char* argv[])
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

