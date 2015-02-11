/// -------------------------------------------------
/// ObjSchedule_main.cpp : 消息调度器对象实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "ObjSchedule_main.h"
#include "Factory_if.h"
#include "BaseID.h"
#include <time.h>
#include "string/tablestring.h"


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CSchedule, "schedule")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CSchedule)
    DCOP_IMPLEMENT_INTERFACE(ISchedule)
    DCOP_IMPLEMENT_INTERFACE(IObject)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 实现对象类
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT(CSchedule)
    DCOP_IMPLEMENT_IDENTIFY_DYNAMIC("name", "id")
    DCOP_IMPLEMENT_CONFIG_THREADSAFE_ENABLE()
    DCOP_IMPLEMENT_CONFIG_STRING("taskname", m_szNameConfig)
    DCOP_IMPLEMENT_CONFIG_INTEGER("taskcount", m_dwCountConfig)
DCOP_IMPLEMENT_IOBJECT_END


/*******************************************************
  函 数 名: CSchedule::CSchedule
  描    述: CSchedule构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CSchedule::CSchedule(Instance *piParent, int argc, char **argv)
{
    (void)snprintf(m_szNameConfig, sizeof(m_szNameConfig), "Sched");
    m_szNameConfig[sizeof(m_szNameConfig) - 1] = '\0';
    m_dwCountConfig = SCHEDULE_COUNT_DEFAULT;

    m_piManager = 0;
    m_pNodes = 0;
    m_dwNodeCount = 0;
    m_dwIdleIndex = 0;

    m_fnMsgProc = 0;
    m_pUserArg = 0;
    m_bOrderReach = true;

    DCOP_CONSTRUCT_INSTANCE(piParent);
    DCOP_CONSTRUCT_IOBJECT(argc, argv);
}

/*******************************************************
  函 数 名: CSchedule::~CSchedule
  描    述: CSchedule析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CSchedule::~CSchedule()
{
    Fini();

    DCOP_DESTRUCT_IOBJECT();
    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CSchedule::Init
  描    述: 初始化入口
  输    入: manager - 对象管理器
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSchedule::Init(IObject *root, int argc, void **argv)
{
    if (!root)
    {
        return FAILURE;
    }

    AutoObjLock(this);

    /// 查询对象
    DCOP_QUERY_OBJECT(IManager, DCOP_OBJECT_MANAGER, root, m_piManager);

    /// 后去配置数量
    DWORD dwCountConfig = m_dwCountConfig;
    if (!dwCountConfig)
    {
        dwCountConfig = SCHEDULE_COUNT_DEFAULT;
    }

    /// 申请调度节点空间
    TaskNode *pTmp = (TaskNode *)DCOP_Malloc(dwCountConfig * sizeof(TaskNode));
    if (!pTmp) return FAILURE;
    (void)memset(pTmp, 0, dwCountConfig * sizeof(TaskNode));

    /// 创建所有的任务
    m_pNodes = pTmp;
    m_dwNodeCount = dwCountConfig;
    DWORD dwRc = CreateAllTask();

    return dwRc;
}

/*******************************************************
  函 数 名: CSchedule::Fini
  描    述: 完成时入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSchedule::Fini()
{
    Enter();
    DestroyAllTask();
    Leave();

    if (m_pNodes)
    {
        DCOP_Free(m_pNodes);
        m_pNodes = 0;
    }

    m_dwNodeCount = 0;
    m_dwIdleIndex = 0;

    DCOP_RELEASE_INSTANCE(m_piManager);
}

/*******************************************************
  函 数 名: CSchedule::Dump
  描    述: DUMP入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSchedule::Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv)
{
    if (!logPrint) return;

    AutoObjLock(this);

    if (!m_pNodes)
    {
        return;
    }

    logPrint(STR_FORMAT("Schedule Dump: (Task Count: %d) (Idle Index: %d) \r\n", m_dwNodeCount, m_dwIdleIndex), logPara);
    CTableString tableStr(5, m_dwNodeCount + 1, "  ");
    tableStr << "InActive";
    tableStr << "LastProcObjID";
    tableStr << "LastCostTime";
    tableStr << "MaxCostTime";
    tableStr << "MaxCostObjID";

    for (DWORD i = 0; i < m_dwNodeCount; ++i)
    {
        tableStr << STR_FORMAT("%d", m_pNodes[i].m_dwInActive);
        tableStr << STR_FORMAT("%d", m_pNodes[i].m_dwLastProcObjID);
        tableStr << STR_FORMAT("%d", m_pNodes[i].m_dwLastCostTimeEnd - m_pNodes[i].m_dwLastCostTimeStart);
        tableStr << STR_FORMAT("%d", m_pNodes[i].m_dwMaxCostTime);
        tableStr << STR_FORMAT("%d", m_pNodes[i].m_dwMaxCostObjID);
    }

    tableStr.Show(logPrint, logPara, "=", "-");
}

/*******************************************************
  函 数 名: CSchedule::SetProc
  描    述: 设置消息处理回调
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSchedule::SetProc(PROC proc, void *arg)
{
    m_fnMsgProc = proc;
    m_pUserArg = arg;
}

/*******************************************************
  函 数 名: CSchedule::SetSameDst
  描    述: 设置是否支持同一目的依次到达
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSchedule::SetSameDst(bool bOrderReach)
{
    m_bOrderReach = bOrderReach;
}

/*******************************************************
  函 数 名: CSchedule::Join
  描    述: 加入调度器
  输    入: 
  输    出: 
  返    回: 成功或者失败的错误码
  修改记录: 
 *******************************************************/
DWORD CSchedule::Join(objMsg *message)
{
    if (!message)
    {
        return FAILURE;
    }

    /// 为了保证发往同一个对象的消息依次到达
    /// 先在接收缓存表中查找队列
    /// 然后再查找空闲的发送队列
    /// 不能依次到达的情况:
    /// 1. 用户设置为禁止
    /// 2. 支持消息并发的对象
    DWORD dwRecvID = message->GetDstID();
    bool bOrderReach = m_bOrderReach;
    if (bOrderReach && m_piManager)
    {
        IObject *piObj = m_piManager->Child(dwRecvID);
        if (piObj && piObj->bConcurrent())
        {
            bOrderReach = false;
        }
    }

    objMsgQue *pMsgQue = GetRecvQue(dwRecvID, bOrderReach);
    if (!pMsgQue)
    {
        pMsgQue = GetSendQue(dwRecvID, bOrderReach);
    }

    if (!pMsgQue)
    {
        return FAILURE;
    }

    /// 进行队列发送，失败要把接收缓存表中的目的ID清除
    DWORD dwRc = pMsgQue->Send(message);
    if (dwRc != SUCCESS)
    {
        DelRecvObj(dwRecvID, bOrderReach);
    }

    return dwRc;
}

/*******************************************************
  函 数 名: CSchedule::CreateTask
  描    述: 创建任务
  输    入: 
  输    出: 
  返    回: 成功或者失败的错误码
  修改记录: 
 *******************************************************/
DWORD CSchedule::CreateAllTask()
{
    /// 由外部进行互斥处理，这里不保护

    if (!m_pNodes)
    {
        return FAILURE;
    }

    for (DWORD i = 0; i < m_dwNodeCount; ++i)
    {
        /// 创建消息队列
        objMsgQue *pMsgQue = DCOP_CreateMsgQue(SCHEDULE_MSGQUE_LEN);
        if (!pMsgQue)
        {
            return FAILURE;
        }

        /// 创建任务
        (void)snprintf(m_pNodes[i].m_szTaskName, sizeof(m_pNodes[i].m_szTaskName), "t%s%u", m_szNameConfig, (i+1));
        m_pNodes[i].m_szTaskName[sizeof(m_pNodes[i].m_szTaskName) - 1] = '\0';
        objTask::IPara *pTaskPara = new TaskPara(this, pMsgQue, i);
        objTask *pTask = DCOP_CreateTask(m_pNodes[i].m_szTaskName,
                        TaskEntry,
                        1024,
                        objTask::OSTASK_PRIORITY_NORMAL,
                        pTaskPara);
        if (!pTask)
        {
            delete pTaskPara;
            delete pMsgQue;
            return FAILURE;
        }

        m_pNodes[i].m_pTask = pTask;
        m_pNodes[i].m_pMsgQue = pMsgQue;
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CSchedule::DestroyAllTask
  描    述: 销毁所有任务
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSchedule::DestroyAllTask()
{
    /// 由外部进行互斥处理，这里不保护

    if (!m_pNodes)
    {
        return;
    }

    for (DWORD i = 0; i < m_dwNodeCount; ++i)
    {
        if (m_pNodes[i].m_pTask)
        {
            delete m_pNodes[i].m_pTask;
            m_pNodes[i].m_pTask = 0;
        }

        if (m_pNodes[i].m_pMsgQue)
        {
            delete m_pNodes[i].m_pMsgQue;
            m_pNodes[i].m_pMsgQue = 0;
        }
    }
}

/*******************************************************
  函 数 名: CSchedule::TaskEntry
  描    述: 任务入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSchedule::TaskEntry(objTask::IPara *para)
{
    TaskPara *pPara = (TaskPara *)para;
    OSASSERT(pPara != NULL);

    CSchedule *pThis = pPara->m_pSchedule;
    if (!pThis)
    {
        return;
    }

    /// 设置当前系统ID到任务变量中
    pThis->SetSystemID();

    objMsgQue *pMsgQue = pPara->m_pMsgQue;
    if (!pMsgQue)
    {
        return;
    }

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
            if (dwRc)
            {
                break;
            }

            if (!message)
            {
                continue;
            }

            DWORD dwProcObjID = message->GetDstID();

            pThis->OnRecvStart(pPara->m_dwIndex, dwProcObjID);
            pThis->OnRecv(message, dwProcObjID);
            pThis->OnRecvEnd(pPara->m_dwIndex, dwProcObjID);

            delete message;
            message = NULL;
        }
    }
}

/*******************************************************
  函 数 名: CSchedule::OnRecvStart
  描    述: 开始接收处理
  输    入: dwIndex     - 索引
            dwProcObjID - 处理的对象ID
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSchedule::OnRecvStart(DWORD dwIndex, DWORD dwProcObjID)
{
    AutoObjLock(this);

    if (!m_pNodes || (dwIndex >= m_dwNodeCount))
    {
        return;
    }

    m_pNodes[dwIndex].m_dwInActive = DCOP_YES;
    m_pNodes[dwIndex].m_dwLastProcObjID = dwProcObjID;
    m_pNodes[dwIndex].m_dwLastCostTimeStart = (DWORD)(clock()/(CLOCKS_PER_SEC/1000));

    objTask *pTask = m_pNodes[dwIndex].m_pTask;
    if (pTask)
    {
        (void)pTask->SetLocal(TASK_LOCAL_HANDLER, &dwProcObjID, sizeof(dwProcObjID));
    }
}

/*******************************************************
  函 数 名: CSchedule::OnRecv
  描    述: 接收处理
  输    入: message     - 消息
            dwProcObjID - 处理的对象ID
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSchedule::OnRecv(objMsg *message, DWORD dwProcObjID)
{
    /// 支持回调的话，直接调用回调进行处理
    if (m_fnMsgProc)
    {
        (*m_fnMsgProc)(message, m_pUserArg);
        return;
    }

    /// 成员m_piManager在处理的任务的生命周期内都不会改变，所以不用保护
    /// 只需要调用目的对象的消息处理函数时进行保护即可(使用目的对象的互斥量)
    if (!m_piManager || !message)
    {
        return;
    }

    /// 获取管理器下面的成员对象
    IObject *pRecvObj = m_piManager->Child(dwProcObjID);
    if (!pRecvObj)
    {
        return;
    }

    /// 这里调用消息入口不进行保护，由对象自己的消息句柄进行保护(见'DCOP_IMPLEMENT_IOBJECT_MSG_HANDLE')
    pRecvObj->Proc(message);

    /// 并发时不能依次到达
    bool bOrderReach = m_bOrderReach;
    if (bOrderReach && pRecvObj->bConcurrent())
    {
        bOrderReach = false;
    }

    /// 删除依次到达的接收缓存
    DelRecvObj(dwProcObjID, bOrderReach);
}

/*******************************************************
  函 数 名: CSchedule::OnRecvEnd
  描    述: 结束接收处理
  输    入: dwIndex     - 索引
            dwProcObjID - 处理的对象ID
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSchedule::OnRecvEnd(DWORD dwIndex, DWORD dwProcObjID)
{
    AutoObjLock(this);

    if (!m_pNodes || (dwIndex >= m_dwNodeCount))
    {
        return;
    }

    m_pNodes[dwIndex].m_dwInActive = DCOP_NO;
    m_pNodes[dwIndex].m_dwLastCostTimeEnd = (DWORD)(clock()/(CLOCKS_PER_SEC/1000));

    DWORD dwCostTime = m_pNodes[dwIndex].m_dwLastCostTimeEnd - m_pNodes[dwIndex].m_dwLastCostTimeStart;
    if (dwCostTime > m_pNodes[dwIndex].m_dwMaxCostTime)
    {
        m_pNodes[dwIndex].m_dwMaxCostTime = dwCostTime;
        m_pNodes[dwIndex].m_dwMaxCostObjID = dwProcObjID;
    }

    objTask *pTask = m_pNodes[dwIndex].m_pTask;
    if (pTask)
    {
        DWORD dwObjID = ID();
        IObject *piParent = Parent();
        if (!dwObjID && piParent)
        {
            dwObjID = piParent->ID();
        }

        (void)pTask->SetLocal(TASK_LOCAL_HANDLER, &dwObjID, sizeof(dwObjID));
    }
}

/*******************************************************
  函 数 名: CSchedule::OnSend
  描    述: 获取一个Idle索引用于发送
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSchedule::OnSend()
{
    /// 由外部进行互斥处理，这里不保护

    if (!m_pNodes)
    {
        return m_dwIdleIndex;
    }

    /// 默认下一个队列
    DWORD dwIdx = m_dwIdleIndex + 1;
    if (dwIdx >= m_dwNodeCount) dwIdx = 0;

    /// 循环找到空闲队列
    DWORD dwIdxTmp = dwIdx;
    DWORD dwCountTmp = (DWORD)(-1);
    while (dwIdx != m_dwIdleIndex)
    {
        /// 获取消息队列
        objMsgQue *pMsgQue = m_pNodes[dwIdx].m_pMsgQue;
        if (!pMsgQue)
        {
            dwIdx++;
            if (dwIdx >= m_dwNodeCount) dwIdx = 0;
            continue;
        }

        /// 如果没有消息缓存和处理，立刻返回
        DWORD dwMsgCount = pMsgQue->Size();
        if (!dwMsgCount && !(m_pNodes[dwIdx].m_dwInActive))
        {
            dwIdxTmp = dwIdx;
            break;
        }

        /// 记录一个最小的消息缓存
        if (dwMsgCount < dwCountTmp)
        {
            dwIdxTmp = dwIdx;
            dwCountTmp = dwMsgCount;
        }

        /// 索引超过范围，进行翻转
        dwIdx++;
        if (dwIdx >= m_dwNodeCount) dwIdx = 0;
    }

    DWORD dwIdxRc = m_dwIdleIndex;
    m_dwIdleIndex = dwIdxTmp;

    return dwIdxRc;
}

/*******************************************************
  函 数 名: CSchedule::GetSendQue
  描    述: 获取空闲的发送消息队列
  输    入: dwObjID     - 对象ID
            bOrderReach - 是否依次到达
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objMsgQue *CSchedule::GetSendQue(DWORD dwObjID, bool bOrderReach)
{
    AutoObjLock(this);

    if (!m_pNodes)
    {
        return NULL;
    }

    DWORD dwIndex = OnSend();

    if (dwIndex >= m_dwNodeCount)
    {
        return NULL;
    }

    if (bOrderReach)
    {
        /// 支持相同目的依次到达时，加入接收缓冲
        RecvObjNode recvNode = {1, dwIndex};
        (void)m_recvobjs.insert(MAP_RECVOBJ::value_type(dwObjID, recvNode));
    }

    return m_pNodes[dwIndex].m_pMsgQue;
}

/*******************************************************
  函 数 名: CSchedule::GetRecvQue
  描    述: 根据对象ID获取缓存的接收消息队列
  输    入: dwObjID     - 对象ID
            bOrderReach - 是否依次到达
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objMsgQue *CSchedule::GetRecvQue(DWORD dwObjID, bool bOrderReach)
{
    AutoObjLock(this);

    if (!bOrderReach)
    {
        /// 不支持相同目的依次到达
        return NULL;
    }

    if (!m_pNodes)
    {
        return NULL;
    }

    IT_RECVOBJ it_recv = m_recvobjs.find(dwObjID);
    if (it_recv == m_recvobjs.end())
    {
        return NULL;
    }

    DWORD dwIndex = ((*it_recv).second).m_dwQueIndex;
    if (dwIndex >= m_dwNodeCount)
    {
        return NULL;
    }

    ((*it_recv).second).m_dwRefCount++;
    if (!((*it_recv).second).m_dwRefCount)
    {
        ((*it_recv).second).m_dwRefCount++;
    }

    return m_pNodes[dwIndex].m_pMsgQue;
}

/*******************************************************
  函 数 名: CSchedule::DelRecvObj
  描    述: 把接收对象ID从接收缓存表中删除
  输    入: dwObjID     - 对象ID
            bOrderReach - 是否依次到达
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSchedule::DelRecvObj(DWORD dwObjID, bool bOrderReach)
{
    AutoObjLock(this);

    if (!bOrderReach)
    {
        /// 不支持相同目的依次到达
        return;
    }

    IT_RECVOBJ it_recv = m_recvobjs.find(dwObjID);
    if (it_recv == m_recvobjs.end())
    {
        return;
    }

    if (((*it_recv).second).m_dwRefCount)
    {
        ((*it_recv).second).m_dwRefCount--;
    }

    if (!((*it_recv).second).m_dwRefCount)
    {
        (void)m_recvobjs.erase(it_recv);
    }
}

/*******************************************************
  函 数 名: CSchedule::SetSystemID
  描    述: 设置系统ID
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CSchedule::SetSystemID()
{
    objTask *pTask = objTask::Current();
    if (!pTask)
    {
        return;
    }

    DWORD dwSysID = 0;
    if (m_piManager)
    {
        dwSysID = m_piManager->GetSystemID();
    }

    DWORD dwRc = pTask->SetLocal(TASK_LOCAL_SYSTEM, &dwSysID, sizeof(dwSysID));
    CHECK_ERRCODE(dwRc, "Set Sys ID To Task Local");

    DWORD dwObjID = ID();
    IObject *piParent = Parent();
    if (!dwObjID && piParent)
    {
        dwObjID = piParent->ID();
    }

    dwRc = pTask->SetLocal(TASK_LOCAL_HANDLER, &dwObjID, sizeof(dwObjID));
    CHECK_ERRCODE(dwRc, "Set Obj ID To Task Local");
}

