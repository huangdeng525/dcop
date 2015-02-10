/// -------------------------------------------------
/// ObjTimer_main.cpp : 对象定时器实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "ObjTimer_main.h"
#include "Factory_if.h"
#include "Manager_if.h"
#include "BaseID.h"


DCOP_IMPLEMENT_FACTORY(CTimer, "timer")

DCOP_IMPLEMENT_INSTANCE(CTimer)
    DCOP_IMPLEMENT_INTERFACE(ITimer)
    DCOP_IMPLEMENT_INTERFACE(IObject)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

DCOP_IMPLEMENT_IOBJECT(CTimer)
    DCOP_IMPLEMENT_IDENTIFY_DYNAMIC("name", "id")
    DCOP_IMPLEMENT_CONFIG_THREADSAFE("threadsafe")
DCOP_IMPLEMENT_IOBJECT_END


/*******************************************************
  函 数 名: CTimer::CTimer
  描    述: CTimer构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CTimer::CTimer(Instance *piParent, int argc, char **argv)
{
    DCOP_CONSTRUCT_INSTANCE(piParent);
    DCOP_CONSTRUCT_IOBJECT(argc, argv);

    m_piDispatch = 0;
    m_pTaskS = 0;
    m_pTaskMs = 0;
}

/*******************************************************
  函 数 名: CTimer::~CTimer
  描    述: CTimer析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CTimer::~CTimer()
{
    Fini();

    DCOP_DESTRUCT_IOBJECT();
    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CTimer::Init
  描    述: 初始化入口
  输    入: root - 对象管理器
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CTimer::Init(IObject *root, int argc, void **argv)
{
    if (!root)
    {
        return FAILURE;
    }

    DCOP_QUERY_OBJECT(IDispatch, DCOP_OBJECT_DISPATCH, root, m_piDispatch);
    if (!m_piDispatch)
    {
        return FAILURE;
    }

    /////////////////////////////////////////////////
    /// 初始化定时器轮
    /////////////////////////////////////////////////

    Enter();

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
    
    m_wheelMs[WHEEL_MS_MILL_SEC].Init(WHEEL_ID_MS_MILL_SEC, 10, 
                            &(m_wheelMs[WHEEL_MS_SEC]), NULL, 
                            TIMER_VALUE_MIN, TimeoutProc, this);
    m_wheelMs[WHEEL_MS_SEC].Init(WHEEL_ID_MS_SEC, 30, 
                            NULL, &(m_wheelMs[WHEEL_MS_MILL_SEC]), 
                            1000, TimeoutProc, this);

    Leave();

    objTask::IPara *pTaskParaS      = new TaskPara(this);
    objTask::IPara *pTaskParaMs     = new TaskPara(this);
    
    m_pTaskS = DCOP_CreateTask("STimerTask",
                STaskEntry,
                1024,
                objTask::OSTASK_PRIORITY_HIGHER,
                pTaskParaS);
    if (!m_pTaskS)
    {
        delete pTaskParaS;
        delete pTaskParaMs;
        return FAILURE;
    }

    m_pTaskMs = DCOP_CreateTask("MsTimerTask",
                MsTaskEntry,
                1024,
                objTask::OSTASK_PRIORITY_HIGHER,
                pTaskParaMs);
    if (!m_pTaskMs)
    {
        delete pTaskParaS;
        delete pTaskParaMs;
        return FAILURE;
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CTimer::Fini
  描    述: 完成时入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CTimer::Fini()
{
    if (m_pTaskMs)
    {
        delete m_pTaskMs;
        m_pTaskMs = 0;
    }

    if (m_pTaskS)
    {
        delete m_pTaskS;
        m_pTaskS = 0;
    }

    DCOP_RELEASE_INSTANCE(m_piDispatch);
}

/*******************************************************
  函 数 名: CTimer::Init
  描    述: 启动一个定时器
  输    入: timerType   - 定时器类型
            dwEvent     - 定时器事件
            dwTimeOut   - 超时时间
            recver      - 接收方
  输    出: 
  返    回: Handle      - 定时器句柄
  修改记录: 
 *******************************************************/
ITimer::Handle CTimer::Start(ITimer::TYPE timerType, DWORD dwEvent, DWORD dwTimeOut, IObject *recver)
{
    if (dwTimeOut < TIMER_VALUE_MIN) dwTimeOut = TIMER_VALUE_MIN;
    TimerValue value = {recver, dwEvent, dwTimeOut, timerType};
    ITimer::Handle hTimer = ITimer::IWheel::Alloc(&value, sizeof(value));
    if (!hTimer)
    {
        return NULL;
    }

    /////////////////////////////////////////////////
    ///     小于30秒，加入毫秒级定时器；否则加入秒级
    /////////////////////////////////////////////////

    Enter();
    DWORD dwRc = InsertToWheel(dwTimeOut, hTimer);
    Leave();

    if (dwRc)
    {
        ITimer::IWheel::Free(hTimer);
        return NULL;
    }

    return hTimer;
}

/*******************************************************
  函 数 名: CTimer::Stop
  描    述: 停止一个定时器
  输    入: hTimer - 定时器句柄
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CTimer::Stop(ITimer::Handle hTimer)
{
    AutoObjLock(this);

    ITimer::IWheel *pWheel = ITimer::IWheel::GetWheel(hTimer);
    if (!pWheel)
    {
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

    ITimer::IWheel::Free(hTimer);
}

/*******************************************************
  函 数 名: CTimer::STaskEntry
  描    述: 秒级定时器任务
  输    入: para - 任务参数
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CTimer::STaskEntry(objTask::IPara *para)
{
    TaskPara *pTaskPara = (TaskPara *)para;
    OSASSERT(pTaskPara != 0);

    CTimer *pThis = pTaskPara->m_pTimer;
    OSASSERT(pThis != 0);

    /// 设置当前系统ID到任务变量中
    pThis->SetSystemID();

    for (;;)
    {
        pThis->Enter();
        pThis->GetSTickBase()->OnTick();
        pThis->Leave();

        objTask::Delay(1000);
    }
}

/*******************************************************
  函 数 名: CTimer::MsTaskEntry
  描    述: 毫秒级定时器任务
  输    入: para - 任务参数
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CTimer::MsTaskEntry(objTask::IPara *para)
{
    TaskPara *pTaskPara = (TaskPara *)para;
    OSASSERT(pTaskPara != 0);

    CTimer *pThis = pTaskPara->m_pTimer;
    OSASSERT(pThis != 0);

    /// 设置当前系统ID到任务变量中
    pThis->SetSystemID();

    for (;;)
    {
        pThis->Enter();
        pThis->GetMsTickBase()->OnTick();
        pThis->Leave();

        objTask::Delay(100);
    }
}

/*******************************************************
  函 数 名: CTimer::TimeoutProc
  描    述: 超时处理函数
  输    入: handle - 定时器句柄
            para   - 回调函数参数
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CTimer::TimeoutProc(ITimer::Handle handle, void *para)
{
    CTimer *pThis = (CTimer *)para;
    OSASSERT(pThis != 0);

    IDispatch *pDispatch = pThis->GetDispatch();
    OSASSERT(pDispatch != 0);

    TimerValue *pValue = (TimerValue *)ITimer::IWheel::GetValue(handle);
    if (!pValue || !pValue->m_dwEvent || !pValue->m_pObject)
    {
        return;
    }

    objMsg *pMsg = DCOP_CreateMsg(0, pValue->m_dwEvent, pThis->ID());
    pMsg->MsgHead().m_dwDstID = pValue->m_pObject->ID();
    DWORD dwRc = pDispatch->Send(pMsg);
    if (dwRc)
    {
        CHECK_RETCODE(dwRc, STR_FORMAT("Send Timeout Msg To Obj:%s Fail(0x%x)!", pValue->m_pObject->Name(), dwRc));
        delete pMsg;
    }

    if (ITimer::TYPE_NOLOOP == pValue->m_timerType)
    {
        return;
    }

    dwRc = pThis->InsertToWheel(pValue->m_dwTimeOut, handle);
    if (dwRc)
    {
        CHECK_RETCODE(dwRc, STR_FORMAT("InsertToWheel(owner:%s) Fail(%d)!", pValue->m_pObject->Name(), dwRc));
    }
}

/*******************************************************
  函 数 名: CTimer::InsertToWheel
  描    述: 插入到定时器轮
  输    入: dwTimeOut   - 超时时间
            hTimer      - 定时器句柄
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CTimer::InsertToWheel(DWORD dwTimeOut, ITimer::Handle hTimer)
{
    DWORD dwRc;

    ITimer::IWheel::SetTimeBase(hTimer, dwTimeOut);

    if ((dwTimeOut >= SEC_TIMER_MAX) || 
        !(dwTimeOut % SEC_TIMER_MIN))
    {
        dwRc = m_wheelS[WHEEL_S_DAY].Add(hTimer);
    }
    else
    {
        dwRc = m_wheelMs[WHEEL_MS_SEC].Add(hTimer);
    }

    return dwRc;
}

/*******************************************************
  函 数 名: CTimer::GetTimeNow
  描    述: 获取定时器当前时间
  输    入: pWheel  - 定时器轮数组
            dwCount - 定时器轮个数
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CTimer::GetTimeNow(CTimerWheel *pWheel, DWORD dwCount)
{
    DWORD dwTime = 0;
    for (DWORD i = 0; i < dwCount; ++i)
    {
        dwTime += pWheel[i].GetScalePoint() * pWheel[i].GetHashBase();
    }

    return dwTime;
}

/*******************************************************
  函 数 名: CTimer::SetSystemID
  描    述: 设置系统ID
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CTimer::SetSystemID()
{
    objTask *pTask = objTask::Current();
    if (!pTask)
    {
        return;
    }

    IObject *piParent = Parent();
    DWORD dwSysID = (piParent)? piParent->ID() : 0;
    DWORD dwObjID = ID();
    DWORD dwRc = pTask->SetLocal(TASK_LOCAL_SYSTEM, &dwSysID, sizeof(dwSysID));
    CHECK_ERRCODE(dwRc, "Set Sys ID To Task Local");
    dwRc = pTask->SetLocal(TASK_LOCAL_HANDLER, &dwObjID, sizeof(dwObjID));
    CHECK_ERRCODE(dwRc, "Set Obj ID To Task Local");
}

