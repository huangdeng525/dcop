/// -------------------------------------------------
/// test_timer.h : 主要测试定时器操作
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TEST_TIMER_H_
#define _TEST_TIMER_H_

#include "test.h"
#include "ObjTimer_if.h"
#include "ObjDispatch_if.h"
#include "ObjTimer_wheel.h"
#include "ObjTimer_main.h"


/// 测试定时器分界值
#define TEST_TIMER_MIN           30000


/// 测试frameworks object attribute
class CTestSuite_TIMER : public ITestSuite
{
public:
    enum WHEEL_S
    {
        WHEEL_S_SEC = 0,
        WHEEL_S_MIN,
        WHEEL_S_HOUR,
        WHEEL_S_DAY
    };

    enum WHEEL_MS
    {
        WHEEL_MS_MILL_SEC = 0,
        WHEEL_MS_SEC
    };

    enum WHEEL_ID
    {
        WHEEL_ID_S_SEC = 1,
        WHEEL_ID_S_MIN,
        WHEEL_ID_S_HOUR,
        WHEEL_ID_S_DAY,
        WHEEL_ID_MS_MILL_SEC,
        WHEEL_ID_MS_SEC
    };

public:
    CTestSuite_TIMER();
    ~CTestSuite_TIMER();

    void BeforeTest();
    int TestEntry(int argc, char* argv[]);

    ITimer::Handle Start(ITimer::TYPE timerType, DWORD dwEvent, DWORD dwTimeOut, IObject *recver);
    void Stop(ITimer::Handle hTimer);

    static void TimeoutProc(ITimer::Handle handle, void *para);

    bool bTickRight(DWORD dwTimeout, DWORD dwTickCnt);

    DWORD InsertToWheel(DWORD dwTimeOut, ITimer::Handle hTimer);
    DWORD GetTimeNow(CTimerWheel *pWheel, DWORD dwCount);

private:
    CTimerWheel m_wheelS[4];        // 秒级定时器
    CTimerWheel m_wheelMs[2];       // 毫秒级定时器
    DWORD m_dwTick;

public:
    ITimer::Handle m_hTimer1;
    ITimer::Handle m_hTimer2;
    ITimer::Handle m_hTimer3;
    DWORD m_dwTickCnt1;
    DWORD m_dwTickCnt2;
    DWORD m_dwTickCnt3;
    DWORD m_dwTickCntBase1;
    DWORD m_dwTickCntBase2;
    DWORD m_dwTickCntBase3;
};


#endif // _TEST_TIMER_H_

