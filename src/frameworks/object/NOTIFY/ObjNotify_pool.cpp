/// -------------------------------------------------
/// ObjNotify_pool.cpp : 通知事件缓冲池实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "ObjNotify_pool.h"
#include "string/tablestring.h"
#include "BaseMessage.h"


/*******************************************************
  函 数 名: CNotifyPool::CNotifyPool
  描    述: CNotifyPool构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CNotifyPool::CNotifyPool()
{
    m_pOwner = 0;
    m_pEventsBuf = 0;
    m_dwEventCount = 0;
    m_fnGetMsgEvent = 0;
}

/*******************************************************
  函 数 名: CNotifyPool::~CNotifyPool
  描    述: CNotifyPool析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CNotifyPool::~CNotifyPool()
{
    m_pOwner = 0;
    
    if (m_pEventsBuf)
    {
        delete [] m_pEventsBuf;
        m_pEventsBuf = 0;
    }

    m_dwEventCount = 0;
}

/*******************************************************
  函 数 名: CNotifyPool::Init
  描    述: 初始化入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CNotifyPool::Init(IObject *owner, DWORD *events, DWORD count, INotify::GET_MSG_EVENT fnGetMsgEvent)
{
    if (m_pEventsBuf || !events || !count)
    {
        return FAILURE;
    }

    Node *pNode = new Node[count];
    if (!pNode)
    {
        return FAILURE;
    }

    (void)memset(pNode, 0, count * sizeof(Node));

    for (DWORD i = 0; i < count; ++i)
    {
        pNode[i].m_dwEvent = events[i];
        pNode[i].m_dwSubscribedCount = 0;
        pNode[i].m_dwProducedCount = 0;
        pNode[i].m_dwDispatchedCount = 0;
    }

    m_pOwner        = owner;
    m_pEventsBuf    = pNode;
    m_dwEventCount  = count;
    m_fnGetMsgEvent = fnGetMsgEvent;

    return SUCCESS;
}

/*******************************************************
  函 数 名: CNotifyPool::OnSubscribe
  描    述: 订阅时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CNotifyPool::OnSubscribe(DWORD event)
{
    AutoObjLock(m_pOwner);

    if (!m_pEventsBuf)
    {
        return FAILURE;
    }

    /////////////////////////////////////////////////
    /// 找出事件，设置被订阅标志
    /// 如果没有被订阅或者没有找到事件，返回失败
    /////////////////////////////////////////////////
    DWORD dwRc = FAILURE;
    for (DWORD i = 0; i < m_dwEventCount; ++i)
    {
        if (m_pEventsBuf[i].m_dwEvent == event)
        {
            if (m_pEventsBuf[i].m_dwSubscribedCount != (DWORD)(-1))
            {
                m_pEventsBuf[i].m_dwSubscribedCount++;
            }

            dwRc = SUCCESS;
            break;
        }
    }

    return dwRc;
}


/*******************************************************
  函 数 名: CNotifyPool::OnPublish
  描    述: 发布时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CNotifyPool::OnPublish(DWORD event)
{
    AutoObjLock(m_pOwner);

    if (!m_pEventsBuf)
    {
        return FAILURE;
    }

    /////////////////////////////////////////////////
    /// 找出事件，检查是否被订阅
    /// 如果没有被订阅或者没有找到事件，返回失败
    /////////////////////////////////////////////////
    DWORD dwRc = FAILURE;
    for (DWORD i = 0; i < m_dwEventCount; ++i)
    {
        if (m_pEventsBuf[i].m_dwEvent == event)
        {
            if (m_pEventsBuf[i].m_dwProducedCount != (DWORD)(-1))
            {
                m_pEventsBuf[i].m_dwProducedCount++;
            }

            if (m_pEventsBuf[i].m_dwSubscribedCount > 0)
            {
                dwRc = SUCCESS;
            }

            break;
        }
    }

    return dwRc;
}

/*******************************************************
  函 数 名: CNotifyPool::IsSubscribedMsgEvent
  描    述: 判断消息是否是被指定消费者订阅的
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool CNotifyPool::IsSubscribedMsgEvent(objMsg *msg, DWORD event)
{
    if (!msg) return false;

    AutoObjLock(m_pOwner);

    if (msg->GetMsgType() != DCOP_MSG_OBJECT_EVENT)
    {
        DWORD dwMsgEventID = (m_fnGetMsgEvent)? (*m_fnGetMsgEvent)(msg) : msg->GetMsgType();
        return (event == dwMsgEventID)? true : false;
    }

    if (msg->GetDataLen() < sizeof(DCOP_SESSION_HEAD))
    {
        return false;
    }

    DCOP_SESSION_HEAD *pSessHead = (DCOP_SESSION_HEAD *)msg->GetDataBuf();
    if (!pSessHead)
    {
        return false;
    }

    return (Bytes_GetDword((BYTE *)&(pSessHead->m_attribute)) == event)? true : false;
}

/*******************************************************
  函 数 名: CNotifyPool::HaveDispatchedMsgEvent
  描    述: 对已经分发的事件进行计数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CNotifyPool::HaveDispatchedMsgEvent(DWORD event)
{
    AutoObjLock(m_pOwner);

    if (!m_pEventsBuf)
    {
        return;
    }

    /////////////////////////////////////////////////
    /// 找出事件，对已经分发的事件进行计数
    /////////////////////////////////////////////////
    for (DWORD i = 0; i < m_dwEventCount; ++i)
    {
        if (m_pEventsBuf[i].m_dwEvent == event)
        {
            if (m_pEventsBuf[i].m_dwDispatchedCount != (DWORD)(-1))
            {
                m_pEventsBuf[i].m_dwDispatchedCount++;
            }

            break;
        }
    }
}

/*******************************************************
  函 数 名: CNotifyPool::Dump
  描    述: Dump
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CNotifyPool::Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv)
{
    if (!logPrint || !m_pOwner) return;

    AutoObjLock(m_pOwner);

    if (!m_pEventsBuf)
    {
        return;
    }

    logPrint(STR_FORMAT("'%s'(%d) Event Pool Dump: (Count: %d) \r\n", m_pOwner->Name(), m_pOwner->ID(), m_dwEventCount), logPara);
    CTableString tableStr(4, m_dwEventCount + 1, "  ");
    tableStr << "event";
    tableStr << "subscribedCount";
    tableStr << "producedCount";
    tableStr << "dispatchedCount";

    for (DWORD i = 0; i < m_dwEventCount; ++i)
    {
        tableStr << STR_FORMAT("0x%x", m_pEventsBuf[i].m_dwEvent);
        tableStr << STR_FORMAT("%d", m_pEventsBuf[i].m_dwSubscribedCount);
        tableStr << STR_FORMAT("%d", m_pEventsBuf[i].m_dwProducedCount);
        tableStr << STR_FORMAT("%d", m_pEventsBuf[i].m_dwDispatchedCount);
    }

    tableStr.Show(logPrint, logPara, "=", "-");
}


