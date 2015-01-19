/// -------------------------------------------------
/// ObjResponse_pool.cpp : 响应缓冲池实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "ObjResponse_pool.h"
#include "Manager_if.h"
#include "Factory_if.h"
#include "ObjAttribute_if.h"


/*******************************************************
  函 数 名: CResponsePool::CResponsePool
  描    述: CResponsePool构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CResponsePool::CResponsePool()
{
    m_pOwner = 0;
    m_pNodeBuf = 0;
    m_dwNodeCount = 0;
    m_dwLastIdx = 0;
    m_piDispatch = 0;
    m_pTimerWheel = 0;
}

/*******************************************************
  函 数 名: CResponsePool::~CResponsePool
  描    述: CResponsePool析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CResponsePool::~CResponsePool()
{
    m_pOwner = 0;

    if (m_pNodeBuf)
    {
        /// 释放队列中的会话消息和定时器句柄
        for (DWORD i = 0; i < m_dwNodeCount; ++i)
        {
            if (m_pNodeBuf[i].m_pSession)
            {
                DCOP_Free(m_pNodeBuf[i].m_pSession);
                m_pNodeBuf[i].m_pSession = 0;
            }
            if (m_pNodeBuf[i].m_hTimer)
            {
                DelFromWheel(m_pNodeBuf[i].m_hTimer);
                ITimer::IWheel::Free(m_pNodeBuf[i].m_hTimer);
                m_pNodeBuf[i].m_hTimer = 0;
            }
        }
    
        delete [] m_pNodeBuf;
        m_pNodeBuf = 0;
    }

    if (m_pTimerWheel)
    {
        delete m_pTimerWheel;
        m_pTimerWheel = 0;
    }

    DCOP_RELEASE_INSTANCE_REFER(m_pOwner, m_piDispatch);
}

/*******************************************************
  函 数 名: CResponsePool::Init
  描    述: 初始化
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CResponsePool::Init(IObject *root, IObject *owner, DWORD count)
{
    if (m_pNodeBuf || !root || !count)
    {
        return FAILURE;
    }

    DCOP_QUERY_OBJECT_REFER(IDispatch, DCOP_OBJECT_DISPATCH, root, owner, m_piDispatch);
    if (!m_piDispatch)
    {
        return FAILURE;
    }

    m_pTimerWheel = ITimer::IWheel::CreateInstance(WHEEL_S_SEC_ID, 
                        WHEEL_S_SEC_SLOT_COUNT, 
                        NULL, NULL, 
                        WHEEL_S_HASH_BASE, 
                        OnTimeout, this);
    if (!m_pTimerWheel)
    {
        return FAILURE;
    }

    Node *pNode = new Node[count];
    if (!pNode)
    {
        return FAILURE;
    }

    (void)memset(pNode, 0, count * sizeof(Node));

    m_pOwner      = owner;
    m_pNodeBuf    = pNode;
    m_dwNodeCount = count;

    m_dwLastIdx = 1;

    return SUCCESS;
}

/*******************************************************
  函 数 名: CResponsePool::GetNode
  描    述: 获取节点
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CResponsePool::Node *CResponsePool::GetNode(DWORD dwIdx)
{
    if (!m_pNodeBuf || !m_dwNodeCount)
    {
        return NULL;
    }

    if (!dwIdx || (dwIdx > m_dwNodeCount))
    {
        return NULL;
    }

    return &(m_pNodeBuf[dwIdx - 1]);
}

/*******************************************************
  函 数 名: CResponsePool::GenIdleIdx
  描    述: 生成空闲索引
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CResponsePool::GenIdleIdx()
{
    if (!m_pNodeBuf || !m_dwNodeCount || !m_dwLastIdx)
    {
        return 0;
    }

    DWORD dwStartIdx = m_dwLastIdx;
    if ((++m_dwLastIdx) > m_dwNodeCount) m_dwLastIdx = 1;

    while (m_dwLastIdx != dwStartIdx)
    {
        if (!(m_pNodeBuf[m_dwLastIdx - 1].m_wNewIdx))
        {
            return m_dwLastIdx;
        }

        if ((++m_dwLastIdx) > m_dwNodeCount) m_dwLastIdx = 1;
    }

    return 0;
}

/*******************************************************
  函 数 名: CResponsePool::OnReq
  描    述: 发送请求时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CResponsePool::OnReq(DCOP_SESSION_HEAD *pSession,
                        DWORD dwMsgType,
                        DWORD dwSrcID,
                        DWORD dwDstID,
                        DWORD dwRspMsgType,
                        DWORD dwTimeout,
                        DWORD dwSendTryTimes)
{
    if (!pSession || (pSession->m_ack != DCOP_REQ) || !dwTimeout || !dwSendTryTimes)
    {
        return FAILURE;
    }

    /// 复制会话消息，以便重传
    DWORD dwSessCopyLen = pSession->m_type.m_headSize + Bytes_GetWord((BYTE *)&(pSession->m_type.m_valueLen));
    DCOP_SESSION_HEAD *pSessCopy = (DCOP_SESSION_HEAD *)DCOP_Malloc(dwSessCopyLen);
    if (!dwSessCopyLen)
    {
        return FAILURE;
    }

    /// 获取一个空闲的索引
    DWORD dwReqIdx = GenIdleIdx();
    Node *pNode = GetNode(dwReqIdx);
    if (!pNode)
    {
        DCOP_Free(pSessCopy);
        return FAILURE;
    }

    /// 判断索引是否真的空闲
    if (pNode->m_wNewIdx)
    {
        DCOP_Free(pSessCopy);
        return FAILURE;
    }

    /// 启动定时器
    TimerValue timerValue = {(WORD)dwReqIdx, Bytes_GetWord((BYTE *)&(pSession->m_index)), dwTimeout, dwSendTryTimes};
    ITimer::Handle hTimer = ITimer::IWheel::Alloc(&timerValue, sizeof(timerValue));
    if (!hTimer)
    {
        DCOP_Free(pSessCopy);
        return FAILURE;
    }

    DWORD dwRc = InsertToWheel(hTimer, dwTimeout);
    if (dwRc != SUCCESS)
    {
        DCOP_Free(pSessCopy);
        ITimer::IWheel::Free(hTimer);
        return dwRc;
    }

    /// 把会话消息中的索引换成新的索引
    /// 接着保存会话的其他参数
    pNode->m_wNewIdx = (WORD)dwReqIdx;
    pNode->m_wOldIdx = Bytes_GetWord((BYTE *)&(pSession->m_index));
    Bytes_SetWord((BYTE *)&(pSession->m_index), (WORD)dwReqIdx);
    (void)memcpy(pSessCopy, pSession, dwSessCopyLen);

    pNode->m_pSession = pSessCopy;
    pNode->m_dwMsgType = dwMsgType;
    pNode->m_dwSrcID = dwSrcID;
    pNode->m_dwDstID = dwDstID;
    pNode->m_dwRspMsgType = dwRspMsgType;
    pNode->m_hTimer = hTimer;

    return SUCCESS;
}

/*******************************************************
  函 数 名: CResponsePool::OnRsp
  描    述: 接收响应时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CResponsePool::OnRsp(DCOP_SESSION_HEAD *pSession)
{
    if (!pSession || !DCOP_RSP(pSession->m_ack))
    {
        return FAILURE;
    }

    /// 获得响应消息中的索引
    DWORD dwReqIdx = Bytes_GetWord((BYTE *)&(pSession->m_index));
    Node *pNode = GetNode(dwReqIdx);
    if (!pNode)
    {
        return FAILURE;
    }

    /// 判断索引是否正确(都是转换后的值，直接比较)
    if ((pNode->m_wNewIdx != dwReqIdx) ||
        (!(pNode->m_pSession)) ||
        (!(pNode->m_hTimer)) ||
        (pNode->m_pSession->m_session != pSession->m_session) ||
        (pNode->m_pSession->m_user != pSession->m_user) ||
        (pNode->m_pSession->m_tty != pSession->m_tty) ||
        (pNode->m_pSession->m_attribute != pSession->m_attribute))
    {
        return FAILURE;
    }

    /// 取得原来的索引
    Bytes_SetWord((BYTE *)&(pSession->m_index), pNode->m_wOldIdx);

    /// 响应持续的话先返回成功，只有结束响应才处理后面的删除动作
    if (pSession->m_ack == DCOP_RSP_CON)
    {
        return SUCCESS;
    }

    /// 停掉并删除定时器
    DelFromWheel(pNode->m_hTimer);
    ITimer::IWheel::Free(pNode->m_hTimer);

    /// 删除缓存的会话消息
    DCOP_Free(pNode->m_pSession);
    (void)memset(pNode, 0, sizeof(Node));

    return SUCCESS;
}

/*******************************************************
  函 数 名: CResponsePool::OnTick
  描    述: 计数处理时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CResponsePool::OnTick()
{
    AutoObjLock(m_pOwner);

    if (!m_pTimerWheel)
    {
        return;
    }

    m_pTimerWheel->OnTick();
}

/*******************************************************
  函 数 名: CResponsePool::OnTimeout
  描    述: 响应超时时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CResponsePool::OnTimeout(ITimer::Handle handle, void *para)
{
    if (!handle || !para)
    {
        return;
    }

    CResponsePool *pThis = (CResponsePool *)para;
    TimerValue *pValue = (TimerValue *)ITimer::IWheel::GetValue(handle);
    DWORD dwTrySendTimes = pThis->TrySendTimes(pValue, handle);
    if (dwTrySendTimes)
    {
        /// 有重试次数，表明重试消息已经发送出去了，不能释放句柄
        return;
    }

    /// 重试次数已经为0，或者发送重试失败，只能释放句柄
    ITimer::IWheel::Free(handle);
}

/*******************************************************
  函 数 名: CResponsePool::InsertToWheel
  描    述: 插入到定时器轮
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CResponsePool::InsertToWheel(ITimer::Handle hTimer, DWORD dwTimeout)
{
    if (dwTimeout)
    {
        ITimer::IWheel::SetTimeBase(hTimer, dwTimeout);
    }

    AutoObjLock(m_pOwner);

    if (!m_pTimerWheel)
    {
        return FAILURE;
    }

    return m_pTimerWheel->Add(hTimer);
}

/*******************************************************
  函 数 名: CResponsePool::DelFromWheel
  描    述: 从定时器轮中删除
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CResponsePool::DelFromWheel(ITimer::Handle hTimer)
{
    AutoObjLock(m_pOwner);

    if (!m_pTimerWheel)
    {
        return;
    }

    m_pTimerWheel->Del(hTimer);
}

/*******************************************************
  函 数 名: CResponsePool::TrySendTimes
  描    述: 尝试重试发送
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CResponsePool::TrySendTimes(TimerValue *pValue, ITimer::Handle hTimer)
{
    if (!m_piDispatch || !pValue || !hTimer)
    {
        return 0;
    }

    /// 获取定时器中的索引
    DWORD dwReqIdx = pValue->m_wNewIdx;
    Node *pNode = GetNode(dwReqIdx);
    if (!pNode)
    {
        return 0;
    }

    /// 检查节点是否正确(可能是别人的Node)
    if ((pNode->m_wNewIdx != dwReqIdx) ||
        (pNode->m_wOldIdx != pValue->m_wOldIdx))
    {
        return 0;
    }

    DCOP_SESSION_HEAD *pSession = 0;
    DWORD dwSessCopyLen = 0;
    objMsg *pMsg = 0;
    DWORD dwRc = SUCCESS;

    /// 检查缓存、有无重发次数
    ERROR_CHECK(pNode->m_pSession != NULL);
    ERROR_CHECK(pValue->m_dwSendTryTimes > 0);

    /// 生成消息
    pSession = pNode->m_pSession;
    dwSessCopyLen = pSession->m_type.m_headSize + Bytes_GetWord((BYTE *)&(pSession->m_type.m_valueLen));
    pMsg = DCOP_LoadMsg(dwSessCopyLen, pNode->m_dwMsgType, pNode->m_dwSrcID, pSession, dwSessCopyLen);
    ERROR_CHECK(pMsg != NULL);

    /// 重启定时器
    dwRc = InsertToWheel(hTimer);
    ERROR_CHECK(dwRc == SUCCESS);

    /// 发送消息
    pMsg->MsgHead().m_dwDstID = pNode->m_dwDstID;
    dwRc = m_piDispatch->Send(pMsg);
    if (dwRc != SUCCESS) delete pMsg;
    ERROR_CHECK(dwRc == SUCCESS);

    /// 发送重试次数(包括本次)
    return (pValue->m_dwSendTryTimes--);


ERROR_LABEL :

    /// 清除缓存
    if (pNode->m_pSession)
    {
        OnRspTimeout(pNode);
        DCOP_Free(pNode->m_pSession);
        pNode->m_pSession = 0;
    }

    /// 删除节点
    (void)memset(pNode, 0, sizeof(Node));

    return 0;
}

/*******************************************************
  函 数 名: CResponsePool::OnRspTimeout
  描    述: 响应超时处理
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CResponsePool::OnRspTimeout(Node *pNode)
{
    if (!pNode || !pNode->m_pSession)
    {
        return;
    }

    CAttribute::PACK_SESS_HEAD sessHead(pNode->m_pSession);
    sessHead.m_ack = DCOP_RSP_END;

    CAttribute::PACK_RSP_HEAD rspHead;
    rspHead.m_retCode = TIMEOUT;

    CDStream sRspParaData;
    CAttribute::PACK_MSG_NODE packNodes[] = 
    {
        {(DCOP_MSG_HEAD *)&rspHead, NULL, 0, sRspParaData},
    };

    (void)IObjectMember::PackMsg(m_piDispatch, NULL, DCOP_OBJECT_RESPONSE, 
                        pNode->m_dwSrcID, pNode->m_dwRspMsgType, 
                        &sessHead, packNodes, ARRAY_SIZE(packNodes));
}

