/// -------------------------------------------------
/// test_timer.cpp : 主要测试定时器操作
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "test_timer.h"


/// -------------------------------------------------
/// TIMER 测试用例
/// -------------------------------------------------
TEST_SUITE_TABLE(TIMER)
    TEST_SUITE_ITEM(CTestSuite_TIMER)
        TEST_CASE_ITEM(4)
            "2100", "2300", "31000", "40"
        TEST_CASE_ITEM_END
        TEST_CASE_ITEM(4)
            "25500", "23000", "61100", "300"
        TEST_CASE_ITEM_END
    TEST_SUITE_ITEM_END
TEST_SUITE_TABLE_END

/// -------------------------------------------------
/// TIMER 测试套
/// -------------------------------------------------
IMPLEMENT_REGTESTSUITE_FUNC(TIMER)


CTestSuite_TIMER::CTestSuite_TIMER()
{
    m_hTimer1 = 0;
    m_hTimer2 = 0;
    m_hTimer3 = 0;
    m_dwTick = 0;
    m_dwTickCnt1 = 0;
    m_dwTickCnt2 = 0;
    m_dwTickCnt3 = 0;
    m_dwTickCntBase1 = 0;
    m_dwTickCntBase2 = 0;
    m_dwTickCntBase3 = 0;
}

CTestSuite_TIMER::~CTestSuite_TIMER()
{
}

void CTestSuite_TIMER::BeforeTest()
{
    m_wheelS[WHEEL_S_SEC].Init(WHEEL_ID_S_SEC, 60, 
                        &(m_wheelS[WHEEL_S_MIN]), NULL, 
                        1000, TimeoutProc, this);
    m_wheelS[WHEEL_S_MIN].Init(WHEEL_ID_S_MIN, 60, 
                        &(m_wheelS[WHEEL_S_HOUR]), &(m_wheelS[WHEEL_S_SEC]), 
                        60000, TimeoutProc, this);
    m_wheelS[WHEEL_S_HOUR].Init(WHEEL_ID_S_HOUR, 24, 
                        &(m_wheelS[WHEEL_S_DAY]), &(m_wheelS[WHEEL_S_MIN]), 
                        3600000, TimeoutProc, this);
    m_wheelS[WHEEL_S_DAY].Init(WHEEL_ID_S_DAY, 49, 
                        NULL, &(m_wheelS[WHEEL_S_HOUR]), 
                        86400000, TimeoutProc, this);
    
    m_wheelMs[WHEEL_MS_MILL_SEC].Init(WHEEL_ID_MS_MILL_SEC, 
                        10, &(m_wheelMs[WHEEL_MS_SEC]), NULL, 
                        TIMER_VALUE_MIN, TimeoutProc, this);
    m_wheelMs[WHEEL_MS_SEC].Init(WHEEL_ID_MS_SEC, 30, 
                        NULL, &(m_wheelMs[WHEEL_MS_MILL_SEC]), 
                        1000, TimeoutProc, this);
}

ITimer::Handle CTestSuite_TIMER::Start(ITimer::TYPE timerType, DWORD dwEvent, DWORD dwTimeOut, IObject *recver)
{
    CTimer::TimerValue value = {recver, dwEvent, dwTimeOut, timerType};
    ITimer::Handle hTimer = ITimer::IWheel::Alloc(&value, sizeof(value));
    if (!hTimer)
    {
        return NULL;
    }

    DWORD dwRc = InsertToWheel(dwTimeOut, hTimer);

    if (dwRc)
    {
        ITimer::IWheel::Free(hTimer);
        return NULL;
    }

    TRACE_LOG(STR_FORMAT("hTimer:%p, Timeout:%d Start!", hTimer, dwTimeOut));
    return hTimer;
}

void CTestSuite_TIMER::Stop(ITimer::Handle hTimer)
{
    ITimer::IWheel *pWheel = ITimer::IWheel::GetWheel(hTimer);
    if (!pWheel)
    {
        TRACE_LOG(STR_FORMAT("hTimer:%p Stop!", hTimer));
        ITimer::IWheel::Free(hTimer);
        return;
    }

    DWORD dwWheelID = pWheel->GetWheelID();

    if ((dwWheelID >= WHEEL_ID_S_SEC) && (dwWheelID <= WHEEL_ID_S_DAY))
    {
        m_wheelS[dwWheelID - WHEEL_ID_S_SEC].Del(hTimer);
    }

    if ((dwWheelID >= WHEEL_ID_MS_MILL_SEC) && (dwWheelID <= WHEEL_ID_MS_SEC))
    {
        m_wheelMs[dwWheelID - WHEEL_ID_MS_MILL_SEC].Del(hTimer);
    }

    TRACE_LOG(STR_FORMAT("hTimer:%p Stop!", hTimer));
    ITimer::IWheel::Free(hTimer);
}

int CTestSuite_TIMER::TestEntry(int argc, char* argv[])
{
    if ((argc < 4) || (!argv))
    {
        return -1;
    }

    DWORD dwTimeout1 = (DWORD)atoi(argv[0]);
    DWORD dwTimeout2 = (DWORD)atoi(argv[1]);
    DWORD dwTimeout3 = (DWORD)atoi(argv[2]);
    DWORD dwTickCnt  = (DWORD)atoi(argv[3]);

    /// 创建定时器1、2、3
    m_hTimer1 = Start(ITimer::TYPE_LOOP,    1, dwTimeout1, (IObject *)1);
    m_hTimer2 = Start(ITimer::TYPE_NOLOOP,  2, dwTimeout2, (IObject *)2);
    m_hTimer3 = Start(ITimer::TYPE_LOOP,    3, dwTimeout3, (IObject *)3);

    m_dwTickCntBase1 = m_dwTick;
    m_dwTickCntBase2 = m_dwTick;
    m_dwTickCntBase3 = m_dwTick;

    /// 递增tick
    DWORD dwTickBase = m_dwTick;
    while (m_dwTick < (dwTickCnt + dwTickBase))
    {
        ++m_dwTick;
        m_wheelS[WHEEL_S_SEC].OnTick();
        m_wheelMs[WHEEL_MS_MILL_SEC].OnTick();
    }

    Stop(m_hTimer1);
    Stop(m_hTimer2);
    Stop(m_hTimer3);

    if (!bTickRight(dwTimeout1, m_dwTickCnt1))
    {
        return -2;
    }

    if (!bTickRight(dwTimeout2, m_dwTickCnt2))
    {
        return -3;
    }

    if (!bTickRight(dwTimeout3, m_dwTickCnt3))
    {
        return -4;
    }

    return 0;
}

void CTestSuite_TIMER::TimeoutProc(ITimer::Handle handle, void *para)
{
    CTestSuite_TIMER *pThis = (CTestSuite_TIMER *)para;
    CTimer::TimerValue *pValue = (CTimer::TimerValue *)ITimer::IWheel::GetValue(handle);

    if (handle == pThis->m_hTimer1)
    {
        TRACE_LOG(STR_FORMAT("Timer1:%p Timeout, TickNow:%d, TickBase:%d", handle, pThis->m_dwTick, pThis->m_dwTickCntBase1));
        pThis->m_dwTickCnt1 = pThis->m_dwTick - pThis->m_dwTickCntBase1;
        pThis->m_dwTickCntBase1 = pThis->m_dwTick;
        (void)pThis->InsertToWheel(pValue->m_dwTimeOut, handle);
    }

    if (handle == pThis->m_hTimer2)
    {
        TRACE_LOG(STR_FORMAT("Timer2:%p Timeout, TickNow:%d, TickBase:%d", handle, pThis->m_dwTick, pThis->m_dwTickCntBase2));
        pThis->m_dwTickCnt2 = pThis->m_dwTick - pThis->m_dwTickCntBase2;
    }

    if (handle == pThis->m_hTimer3)
    {
        TRACE_LOG(STR_FORMAT("Timer3:%p Timeout, TickNow:%d, TickBase:%d", handle, pThis->m_dwTick, pThis->m_dwTickCntBase3));
        pThis->m_dwTickCnt3 = pThis->m_dwTick - pThis->m_dwTickCntBase3;
        pThis->m_dwTickCntBase3 = pThis->m_dwTick;
        (void)pThis->InsertToWheel(pValue->m_dwTimeOut, handle);
    }
}

bool CTestSuite_TIMER::bTickRight(DWORD dwTimeout, DWORD dwTickCnt)
{
    if (dwTimeout >= TEST_TIMER_MIN)
    {
        dwTimeout /= 1000;
    }
    else
    {
        dwTimeout /= 100;
    }

    if (dwTimeout == dwTickCnt)
    {
        return true;
    }

   return false;
}

DWORD CTestSuite_TIMER::InsertToWheel(DWORD dwTimeOut, ITimer::Handle hTimer)
{
    DWORD dwRc;

    ITimer::IWheel::SetTimeBase(hTimer, dwTimeOut);

    if (dwTimeOut >= TEST_TIMER_MIN)
    {
        dwRc = m_wheelS[WHEEL_S_DAY].Add(hTimer);
    }
    else
    {
        dwRc = m_wheelMs[WHEEL_MS_SEC].Add(hTimer);
    }

    return dwRc;
}

DWORD CTestSuite_TIMER::GetTimeNow(CTimerWheel *pWheel, DWORD dwCount)
{
    DWORD dwTime = 0;
    for (DWORD i = 0; i < dwCount; ++i)
    {
        dwTime += pWheel[i].GetScalePoint() * pWheel[i].GetHashBase();
    }

    return dwTime;
}

