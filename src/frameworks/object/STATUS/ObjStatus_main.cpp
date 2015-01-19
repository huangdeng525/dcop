/// -------------------------------------------------
/// ObjStatus_main.cpp : 状态机管理实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "ObjStatus_main.h"
#include "Factory_if.h"
#include "Manager_if.h"
#include "BaseMessage.h"


DCOP_IMPLEMENT_FACTORY(CStatus, "status")

DCOP_IMPLEMENT_INSTANCE(CStatus)
    DCOP_IMPLEMENT_INTERFACE(IStatus)
    DCOP_IMPLEMENT_INTERFACE(IObject)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

DCOP_IMPLEMENT_IOBJECT(CStatus)
    DCOP_IMPLEMENT_IDENTIFY_DYNAMIC("name", "id")
    DCOP_IMPLEMENT_CONFIG_THREADSAFE("threadsafe")
DCOP_IMPLEMENT_IOBJECT_END


/*******************************************************
  函 数 名: CStatus::CStatus
  描    述: CStatus构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CStatus::CStatus(Instance *piParent, int argc, char **argv)
{
    DCOP_CONSTRUCT_INSTANCE(piParent);
    DCOP_CONSTRUCT_IOBJECT(argc, argv);

    m_pStateNodes = 0;
    m_dwStateCount = 0;
    m_dwCurState = Invalid;
    m_bRunning = false;
}

/*******************************************************
  函 数 名: CStatus::~CStatus
  描    述: CTimer析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CStatus::~CStatus()
{
    if (m_pStateNodes)
    {
        DCOP_Free(m_pStateNodes);
        m_pStateNodes = 0;
    }

    m_dwStateCount = 0;
    m_dwCurState = Invalid;
    m_bRunning = false;

    DCOP_DESTRUCT_IOBJECT();
    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CStatus::Reg
  描    述: 注册状态机
  输    入: pObjOwner       - 所有者
            pStateNodes     - 状态列表
            dwStateCount    - 状态个数
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CStatus::Reg(REG *pStateNodes, DWORD dwStateCount)
{
    if (!pStateNodes || !dwStateCount)
    {
        return FAILURE;
    }

    AutoObjLock(this);

    if (m_bRunning)
    {
        return FAILURE;
    }

    /////////////////////////////////////////////////
    /// 1.申请一片新的缓冲区
    /////////////////////////////////////////////////
    DWORD dwStateCountTmp = m_dwStateCount + dwStateCount;
    STATE *pStateNodesTmp = (STATE *)DCOP_Malloc(dwStateCountTmp * sizeof(STATE));
    if (!pStateNodesTmp)
    {
        return FAILURE;
    }

    /////////////////////////////////////////////////
    /// 2.复制以前的注册项(支持分开多次注册)
    /////////////////////////////////////////////////
    if (m_pStateNodes)
    {
        (void)memcpy(pStateNodesTmp, m_pStateNodes, m_dwStateCount * sizeof(STATE));
        DCOP_Free(m_pStateNodes);
        m_pStateNodes = 0;
    }

    /////////////////////////////////////////////////
    /// 3.复制新的注册项
    /////////////////////////////////////////////////
    for (DWORD i = 0; i < dwStateCount; ++i)
    {
        (void)memcpy(pStateNodesTmp + m_dwStateCount + i, pStateNodes + i, sizeof(REG));
        pStateNodesTmp[dwStateCount + i].m_dwRunStateValue = Invalid;
        pStateNodesTmp[dwStateCount + i].m_dwRunPrevState = Invalid;
        pStateNodesTmp[dwStateCount + i].m_dwRunTimeValue = 0;
    }

    m_pStateNodes = pStateNodesTmp;
    m_dwStateCount = dwStateCountTmp;

    return SUCCESS;
}

/*******************************************************
  函 数 名: CStatus::Start
  描    述: 开始状态机
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CStatus::Start()
{
    AutoObjLock(this);

    if (m_bRunning)
    {
        return FAILURE;
    }

    /////////////////////////////////////////////////
    /// 触发DCOP_MSG_STATUS_START事件
    /////////////////////////////////////////////////
    DWORD dwRc = EventToOwner(DCOP_MSG_STATUS_START);
    if (dwRc)
    {
        return FAILURE;
    }

    m_bRunning = true;
    return SUCCESS;
}

/*******************************************************
  函 数 名: CStatus::Stop
  描    述: 停止状态机
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CStatus::Stop()
{
    AutoObjLock(this);

    if (!m_bRunning)
    {
        return FAILURE;
    }

    /////////////////////////////////////////////////
    /// 触发DCOP_MSG_STATUS_STOP事件
    /////////////////////////////////////////////////
    DWORD dwRc = EventToOwner(DCOP_MSG_STATUS_STOP);
    if (dwRc)
    {
        return FAILURE;
    }

    m_bRunning = false;
    return SUCCESS;
}

/*******************************************************
  函 数 名: CStatus::Tick
  描    述: 计时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CStatus::Tick()
{
    AutoObjLock(this);

    if (!m_bRunning || !m_pStateNodes || !m_dwStateCount || (m_dwCurState >= m_dwStateCount))
    {
        return FAILURE;
    }

    /////////////////////////////////////////////////
    /// 1.进行计时器计时
    /////////////////////////////////////////////////
    m_pStateNodes[m_dwCurState].m_dwRunTimeValue++;
    if (OSWAIT_FOREVER == m_pStateNodes[m_dwCurState].m_dwTimeout)
    {
        return SUCCESS;
    }

    /////////////////////////////////////////////////
    /// 2.没有超时直接返回成功
    /////////////////////////////////////////////////
    DWORD dwTimeValue = m_pStateNodes[m_dwCurState].m_dwRunTimeValue;
    if (dwTimeValue < m_pStateNodes[m_dwCurState].m_dwTimeout)
    {
        return SUCCESS;
    }

    /////////////////////////////////////////////////
    /// 3.超时的触发DCOP_MSG_STATUS_TIMEOUT事件
    /////////////////////////////////////////////////
    DWORD dwRc = EventToOwner(DCOP_MSG_STATUS_TIMEOUT);
    if (dwRc)
    {
        return FAILURE;
    }

    /////////////////////////////////////////////////
    /// 4.计时器清零，如果超时状态不可用或者不变，直接返回成功
    /////////////////////////////////////////////////
    m_pStateNodes[m_dwCurState].m_dwRunTimeValue = 0;
    DWORD dwBackState = m_pStateNodes[m_dwCurState].m_dwBackStateWhenTimeout;
    if ((dwBackState >= m_dwStateCount) || (dwBackState == m_dwCurState))
    {
        return SUCCESS;
    }

    /////////////////////////////////////////////////
    /// 5.进入设定的超时状态
    /////////////////////////////////////////////////
    dwRc = SetNewState(dwBackState);
    return dwRc;
}

/*******************************************************
  函 数 名: CStatus::SetNewState
  描    述: 进入新的状态
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CStatus::SetNewState(DWORD dwNewState)
{
    AutoObjLock(this);

    if (!m_bRunning || !m_pStateNodes || !m_dwStateCount || (m_dwCurState >= m_dwStateCount))
    {
        return FAILURE;
    }

    /////////////////////////////////////////////////
    /// 触发DCOP_MSG_STATUS_NEW事件
    /////////////////////////////////////////////////
    DWORD dwRc = EventToOwner(DCOP_MSG_STATUS_NEW);
    if (dwRc)
    {
        return FAILURE;
    }

    if (dwNewState >= m_dwStateCount)
    {
        dwNewState = m_pStateNodes[m_dwCurState].m_dwNextStateDefault;
    }

    m_dwCurState = dwNewState;
    return SUCCESS;
}

/*******************************************************
  函 数 名: CStatus::GetCurState
  描    述: 获取当前状态
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CStatus::GetCurState()
{
    AutoObjLock(this);

    return m_dwCurState;
}

/*******************************************************
  函 数 名: CStatus::GetStateInfo
  描    述: 获取状态信息
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
IStatus::STATE *CStatus::GetStateInfo(DWORD dwState)
{
    AutoObjLock(this);

    if (!m_pStateNodes || !m_dwStateCount || (dwState >= m_dwStateCount))
    {
        return NULL;
    }

    return &(m_pStateNodes[dwState]);
}

/*******************************************************
  函 数 名: CStatus::EventToOwner
  描    述: 发送事件给所有者
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CStatus::EventToOwner(DWORD event)
{
    if (!m_piParent)
    {
        return FAILURE;
    }

    /////////////////////////////////////////////////
    /// 1.转换父对象为所有者
    /////////////////////////////////////////////////
    IObject *pOwner = 0; 
    if (m_piParent->QueryInterface(ID_INTF(IManager), REF_PTR(pOwner), this) != SUCCESS)
    {
        return FAILURE;
    }

    if (!pOwner)
    {
        return FAILURE;
    }

    /////////////////////////////////////////////////
    /// 2.直接调用所有者处理接口
    /////////////////////////////////////////////////
    objMsg *pMsg = DCOP_CreateMsg(0, event, ID());
    if (!pMsg)
    {
        (void)pOwner->Release(this);
        return FAILURE;
    }

    pOwner->Proc(pMsg);
    (void)pOwner->Release(this);
    delete pMsg;

    return SUCCESS;
}

