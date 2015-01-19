/// -------------------------------------------------
/// ObjTimer_main.h : 对象定时器私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJTIMER_MAIN_H_
#define _OBJTIMER_MAIN_H_

#include "ObjTimer_if.h"
#include "ObjDispatch_if.h"
#include "ObjTimer_wheel.h"
#include "task.h"


/// 定时器选择秒级和毫秒级的最大值/最小值
#define SEC_TIMER_MAX           30000
#define SEC_TIMER_MIN           1000


/// 定时器实现类
class CTimer : public ITimer
{
public:
    /// 定时器任务参数
    struct TaskPara : public objTask::IPara
    {
        CTimer *m_pTimer;

        TaskPara()
        {
            m_pTimer = 0;
        }

        TaskPara(CTimer *pTimer)
        {
            m_pTimer = pTimer;
        }

        ~TaskPara()
        {
            m_pTimer = 0;
        }
    };

    /// 秒级定时器
    enum WHEEL_S
    {
        WHEEL_S_SEC = 0,
        WHEEL_S_MIN,
        WHEEL_S_HOUR,
        WHEEL_S_DAY
    };

    /// 毫秒级定时器
    enum WHEEL_MS
    {
        WHEEL_MS_MILL_SEC = 0,
        WHEEL_MS_SEC
    };

    /// 定时器ID
    enum WHEEL_ID
    {
        WHEEL_ID_S_SEC = 1,
        WHEEL_ID_S_MIN,
        WHEEL_ID_S_HOUR,
        WHEEL_ID_S_DAY,
        WHEEL_ID_MS_MILL_SEC,
        WHEEL_ID_MS_SEC
    };

    /// TimerValue是注册的定时器信息
    struct TimerValue
    {
        IObject *m_pObject;                         // 对象
        DWORD m_dwEvent;                            // 事件
        DWORD m_dwTimeOut;                          // 超时时间
        ITimer::TYPE m_timerType;                   // 定时器类型
    };
    
public:
    CTimer(Instance *piParent, int argc, char **argv);
    ~CTimer();

    DCOP_DECLARE_INSTANCE;
    DCOP_DECLARE_IOBJECT;

    DWORD Init(IObject *root, int argc, void **argv);
    void Fini();

    ITimer::Handle Start(ITimer::TYPE timerType, DWORD dwEvent, DWORD dwTimeOut, IObject *recver);

    void Stop(ITimer::Handle hTimer);

    static void STaskEntry(objTask::IPara *para);
    static void MsTaskEntry(objTask::IPara *para);
    static void TimeoutProc(ITimer::Handle handle, void *para);

    CTimerWheel *GetSTickBase() {return &(m_wheelS[WHEEL_S_SEC]);}
    CTimerWheel *GetMsTickBase() {return &(m_wheelMs[WHEEL_MS_MILL_SEC]);}

    IDispatch *GetDispatch() {return m_piDispatch;}

    DWORD InsertToWheel(DWORD dwTimeOut, ITimer::Handle hTimer);
    DWORD GetTimeNow(CTimerWheel *pWheel, DWORD dwCount);

private:
    IDispatch * m_piDispatch;       // 消息发送器
    objTask *   m_pTaskS;           // 秒级任务
    CTimerWheel m_wheelS[4];        // 秒级定时器
    objTask *   m_pTaskMs;          // 毫秒级任务
    CTimerWheel m_wheelMs[2];       // 毫秒级定时器
};


#endif // #ifndef _OBJTIMER_MAIN_H_

