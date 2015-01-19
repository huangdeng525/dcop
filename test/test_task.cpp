/// -------------------------------------------------
/// test_task.cpp : Ö÷Òª²âÊÔwrapper task²Ù×÷
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "test_task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <errno.h>


/// -------------------------------------------------
/// TASK ²âÊÔÓÃÀý
/// -------------------------------------------------
TEST_SUITE_TABLE(TASK)
    TEST_SUITE_ITEM(CTestSuiteTask)
        TEST_CASE_ITEM(2)
            "1", "10"
        TEST_CASE_ITEM_END
        TEST_CASE_ITEM(2)
            "10", "100"
        TEST_CASE_ITEM_END
        TEST_CASE_ITEM(2)
            "100", "1000"
        TEST_CASE_ITEM_END
        TEST_CASE_ITEM(2)
            "1", "1000"
        TEST_CASE_ITEM_END
    #if 1
        TEST_CASE_ITEM(2)
            "10", "1000"
        TEST_CASE_ITEM_END
    #endif
    TEST_SUITE_ITEM_END
TEST_SUITE_TABLE_END

/// -------------------------------------------------
/// TASK ²âÊÔÌ×
/// -------------------------------------------------
IMPLEMENT_REGTESTSUITE_FUNC(TASK)


struct TestTaskPara : public objTask::IPara
{
    DWORD m_dwNum;
    objMsgQue *m_pMsgQue;
    objCounter *m_pCountSem;
    objLock *m_pLock;
    int m_iSenderCount;
    int m_iRecverCount;
    CTestSuiteTask *m_pTestSuite;

    TestTaskPara()
    {
        m_dwNum = 0;
        m_pMsgQue = 0;
        m_pCountSem = 0;
        m_pLock = 0;
        m_iSenderCount = 0;
        m_iRecverCount = 0;
        m_pTestSuite = 0;
    }

    TestTaskPara(CTestSuiteTask *pTestSuite, DWORD dwNum, objMsgQue *pMsgQue, 
                objLock *pLock = 0, objCounter *pCountSem = 0, 
                int iSenderCount = 0, int iRecverCount = 0)
    {
        m_dwNum = dwNum;
        m_pMsgQue = pMsgQue;
        m_pCountSem = pCountSem;
        m_pLock = pLock;
        m_iSenderCount = iSenderCount;
        m_iRecverCount = iRecverCount;
        m_pTestSuite = pTestSuite;
    }

    ~TestTaskPara()
    {
        m_dwNum = 0;
        m_pMsgQue = 0;
        m_pCountSem = 0;
        m_pLock = 0;
        m_iSenderCount = 0;
        m_iRecverCount = 0;
        m_pTestSuite = 0;
    }
};


CTestSuiteTask::CTestSuiteTask()
{
    m_iSenderCount = 0;
    m_iRecverCount = 0;
}

CTestSuiteTask::~CTestSuiteTask()
{
    m_iSenderCount = 0;
    m_iRecverCount = 0;
}

int CTestSuiteTask::TestEntry(int argc, char* argv[])
{
    if ((argc < 2) || (!argv))
    {
        return -1;
    }

    int itestSenderCount = atoi(argv[0]);
    if (itestSenderCount <= 0)
    {
        return -2;
    }

    int itestRecverCount = atoi(argv[1]);
    if (itestRecverCount <= 0)
    {
        return -3;
    }

    objMsgQue *m_pMsgQue;
    objCounter *m_pCountSem;
    objLock *m_pLock;

    m_pMsgQue = DCOP_CreateMsgQue(itestRecverCount);
    TRACE_LOG(STR_FORMAT("Task Test Instance m_pMsgQue(%p).", m_pMsgQue));
    assert(m_pMsgQue);

    m_pCountSem = DCOP_CreateCounter(itestSenderCount, itestSenderCount);
    TRACE_LOG(STR_FORMAT("Task Test Instance m_pCountSem(%p).", m_pCountSem));
    assert(m_pCountSem);

    m_pLock = DCOP_CreateLock();
    TRACE_LOG(STR_FORMAT("Task Test Instance m_pLock(%p).", m_pLock));
    assert(m_pLock);


    TRACE_LOG("Task Test Start ...");


    objTask *pRecvTask = DCOP_CreateTask("TestRecvTask", 
                            task_entry_recv, 
                            1024, 
                            objTask::OSTASK_PRIORITY_NORMAL, 
                            new TestTaskPara(this, 0, m_pMsgQue, m_pLock));

    for (int i = 0; i < itestSenderCount; ++i)
    {
        DCOP_CreateTask("TestSendTask", 
                            task_entry_send, 
                            1024, 
                            objTask::OSTASK_PRIORITY_NORMAL, 
                            new TestTaskPara(this, i + 1, m_pMsgQue, m_pLock, m_pCountSem, itestSenderCount, itestRecverCount));
    }

    for (;;)
    {

        m_pLock->Enter();
        DWORD dwRc = m_pCountSem->Take(0);
        if (dwRc)
        {
            TRACE_LOG(STR_FORMAT("Count Sem Take Fail(0x%x)", dwRc));
            m_pLock->Leave();
            break;
        }

        (void)m_pCountSem->Give();
        m_pLock->Leave();

        objTask::Delay(1000);
        continue;
    }

    TRACE_LOG("Test Task Entry Loop Over");

    delete pRecvTask;
    pRecvTask = 0;

    if (m_pLock)
    {
        TRACE_LOG(STR_FORMAT("Task Test delete m_pLock(%p).", m_pLock));
        m_pLock->Enter();
        m_pLock->Leave();
        delete m_pLock;
        m_pLock = 0;
    }

    if (m_pCountSem)
    {
        TRACE_LOG(STR_FORMAT("Task Test delete m_pCountSem(%p).", m_pCountSem));
        delete m_pCountSem;
        m_pCountSem = 0;
    }

    if (m_pMsgQue)
    {
        TRACE_LOG(STR_FORMAT("Task Test delete m_pMsgQue(%p).", m_pMsgQue));
        delete m_pMsgQue;
        m_pMsgQue = 0;
    }

    TRACE_LOG(STR_FORMAT("Task Test End(send:%d,recv:%d)!", m_iSenderCount, m_iRecverCount));

    return ((!m_iSenderCount) || (!m_iRecverCount) || (m_iSenderCount != m_iRecverCount))? -4 : 0;

}

void CTestSuiteTask::task_entry_recv(objTask::IPara *para)
{
    /// printf("recv task start!\n");

    TestTaskPara *pTaskPara = (TestTaskPara *)para;
    assert(pTaskPara);

    objMsgQue *pMsgQue = pTaskPara->m_pMsgQue;
    assert(pMsgQue);

    objLock *pLock = pTaskPara->m_pLock;
    assert(pLock);

    assert(pTaskPara->m_pTestSuite);

    for (;;)
    {
        DWORD dwRc = pMsgQue->Wait(OSWAIT_FOREVER);
        if (dwRc)
        {
            continue;
        }

        for (;;)
        {
            objMsg *message = NULL;
            dwRc = pMsgQue->Recv(message);
            if (dwRc || !message)
            {
                break;
            }

            pLock->Enter();
            pTaskPara->m_pTestSuite->m_iRecverCount++;
            pLock->Leave();

        #if 0
            time_t ltime;
            time(&ltime);
            printf("task recv: %s |%lu_%lu| at %ld, left:%lu.\n", 
                (char *)(message->GetDataBuf()), 
                message->GetMsgHead()->m_dwMsgType,
                message->GetMsgHead()->m_dwTransNo,
                ltime,
                pMsgQue->Size());
        #endif

            delete message;
        }

    }
}

void CTestSuiteTask::task_entry_send(objTask::IPara *para)
{
    TestTaskPara *pTaskPara = (TestTaskPara *)para;
    assert(pTaskPara);

    objMsgQue *pMsgQue = pTaskPara->m_pMsgQue;
    assert(pMsgQue);

    objCounter *pCountSem = pTaskPara->m_pCountSem;
    assert(pCountSem);

    objLock *pLock = pTaskPara->m_pLock;
    assert(pLock);

    assert(pTaskPara->m_pTestSuite);

    DWORD number = pTaskPara->m_dwNum;
    DWORD count = 0;

    TRACE_LOG(STR_FORMAT("send task(%lu) start!", number));

    for (;;)
    {
        srand((unsigned int)time(0));
        objTask::Delay(rand() % 100 + 10);

        assert(pMsgQue);

        objMsg *message = DCOP_CreateMsg(100, number, ++count);
        if (!message)
        {
            CHECK_RETCODE(FAILURE, STR_FORMAT("t%lu malloc error(%s)!", number, strerror(errno)));
            continue;
        }

    #if 0
        time_t ltime;
        time(&ltime);
        sprintf((char *)(message->GetDataBuf()), 
                "I am t%lu [%lu] <%ld>", 
                number, 
                count, 
                ltime);
    #endif

        if (pMsgQue->Send(message))
        {
            delete message;
            objTask::Delay(1000);
        }
        else
        {
            pLock->Enter();
            pTaskPara->m_pTestSuite->m_iSenderCount++;
            pLock->Leave();
        }

        if (count >= (DWORD)(pTaskPara->m_iRecverCount/pTaskPara->m_iSenderCount + 1))
        {
            pLock->Enter();
            (void)pCountSem->Take(0);
            pLock->Leave();
            break;
        }
    }

    TRACE_LOG(STR_FORMAT("send task(%lu) end!", number));
}

