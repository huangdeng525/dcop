/// -------------------------------------------------
/// ObjNotify_main.cpp : 订阅发布器对象实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "ObjNotify_main.h"
#include "Factory_if.h"
#include "Manager_if.h"
#include "ObjDispatch_if.h"
#include "BaseID.h"
#include "string/tablestring.h"


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CNotify, "notify")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CNotify)
    DCOP_IMPLEMENT_INTERFACE(INotify)
    DCOP_IMPLEMENT_INTERFACE(IObject)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 实现对象类
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT(CNotify)
    DCOP_IMPLEMENT_IDENTIFY_DYNAMIC("name", "id")
    DCOP_IMPLEMENT_CONFIG_THREADSAFE("threadsafe")
DCOP_IMPLEMENT_IOBJECT_END


/*******************************************************
  函 数 名: CNotify::CNotify
  描    述: CNotify构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CNotify::CNotify(Instance *piParent, int argc, char **argv)
{
    DCOP_CONSTRUCT_INSTANCE(piParent);
    DCOP_CONSTRUCT_IOBJECT(argc, argv);

    m_SubscribeTmp.SetNodeSize(sizeof(SubscribeNode));

    m_piDispatch = 0;
}

/*******************************************************
  函 数 名: CNotify::~CNotify
  描    述: CNotify析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CNotify::~CNotify()
{
    Fini();

    DCOP_DESTRUCT_IOBJECT();
    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CNotify::Init
  描    述: 初始化入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CNotify::Init(IObject *root, int argc, void **argv)
{
    if (!root)
    {
        return FAILURE;
    }

    AutoObjLock(this);

    /// 查询分发器
    DCOP_QUERY_OBJECT(IDispatch, DCOP_OBJECT_DISPATCH, root, m_piDispatch);
    if (!m_piDispatch)
    {
        return FAILURE;
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CNotify::Fini
  描    述: 完成时入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CNotify::Fini()
{
    DCOP_RELEASE_INSTANCE(m_piDispatch);
}

/*******************************************************
  函 数 名: CNotify::Proc
  描    述: 消息处理
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CNotify::Proc(objMsg *msg)
{
    if (!msg)
    {
        return;
    }

    AutoObjLock(this);

    DWORD producerID = msg->GetSrcID();
    IT_NOTIFY it = m_events.find(producerID);
    if (it == m_events.end())
    {
        return;
    }

    CNotifyPool *pPool = &((*it).second);

    /////////////////////////////////////////////////
    /// 从缓存的订阅查找并分发给消费者
    /////////////////////////////////////////////////
    for (DWORD i = 0; i < m_SubscribeTmp.Count(); ++i)
    {
        SubscribeNode *pNode = (SubscribeNode *)m_SubscribeTmp.Pos(i);
        if (!pNode)
        {
            continue;
        }

        /// 不是订阅的该生产者的事件，返回
        if (pNode->producer != producerID)
        {
            continue;
        }

        /// 不是订阅的该消息中的事件，返回
        if (!pPool->IsSubscribedMsgEvent(msg, pNode->event))
        {
            continue;
        }

        /// 复制消息，并分发给订阅的消费者
        objMsg *msgNew = DCOP_CloneMsg(msg);
        if (!msgNew)
        {
            continue;
        }

        /// 分发事件
        msgNew->MsgHead().m_dwDstID = pNode->consumer;
        DWORD dwRc = m_piDispatch->Send(msgNew);
        if (dwRc)
        {
            CHECK_RETCODE(dwRc, STR_FORMAT("Notify Dispatch Event:0x%x To ObjID:%d Fail!", 
                        pNode->event, pNode->consumer));
            delete msgNew;
        }

        /// 对已分发的事件进行计数
        pPool->HaveDispatchedMsgEvent(pNode->event);
    }
}

/*******************************************************
  函 数 名: CNotify::Dump
  描    述: Dump
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CNotify::Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv)
{
    if (!logPrint) return;

    AutoObjLock(this);

    logPrint(STR_FORMAT("Subscribe Dump: (Count: %d) \r\n", m_SubscribeTmp.Count()), logPara);
    CTableString tableStr(3, m_SubscribeTmp.Count() + 1, "  ");
    tableStr << "consumer";
    tableStr << "producer";
    tableStr << "event";

    for (DWORD i = 0; i < m_SubscribeTmp.Count(); ++i)
    {
        SubscribeNode *pNode = (SubscribeNode *)m_SubscribeTmp.Pos(i);
        if (!pNode)
        {
            continue;
        }

        tableStr << STR_FORMAT("%d", pNode->consumer);
        tableStr << STR_FORMAT("%d", pNode->producer);
        tableStr << STR_FORMAT("0x%x", pNode->event);
    }

    tableStr.Show(logPrint, logPara, "=", "-");

    for (IT_NOTIFY it = m_events.begin(); it != m_events.end(); ++it)
    {
        ((*it).second).Dump(logPrint, logPara, argc, argv);
    }
}

/*******************************************************
  函 数 名: CNotify::CreatePool
  描    述: 创建缓冲池
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
INotify::IPool *CNotify::CreatePool(IObject *producer, DWORD *events, DWORD count, GET_MSG_EVENT fnGetMsgEvent)
{
    if (!producer) return NULL;

    AutoObjLock(this);

    IT_NOTIFY it = m_events.find(producer->ID());
    if (it != m_events.end())
    {
        return &((*it).second);
    }

    CNotifyPool pool;
    it = m_events.insert(m_events.end(), MAP_NOTIFY::value_type(producer->ID(), pool));
    if (it == m_events.end())
    {
        return NULL;
    }

    CNotifyPool *pPool = &((*it).second);
    DWORD dwRc = pPool->Init(producer, events, count, fnGetMsgEvent);
    if (dwRc != SUCCESS)
    {
        (void)m_events.erase(it);
        return NULL;
    }

    ProcTmpOfSubscribe(pPool);
    return pPool;
}

/*******************************************************
  函 数 名: CNotify::DestroyPool
  描    述: 删除缓冲池
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CNotify::DestroyPool(IPool *pool)
{
    if (!pool) return;

    AutoObjLock(this);

    CNotifyPool *pPool = (CNotifyPool *)pool;
    IObject *pProducer = pPool->GetProducer();
    if (!pProducer)
    {
        return;
    }

    (void)m_events.erase(pProducer->ID());
}

/*******************************************************
  函 数 名: CNotify::Subscribe
  描    述: 订阅
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CNotify::Subscribe(DWORD consumer, DWORD producer, DWORD event)
{
    AutoObjLock(this);

    /// 先进行本地缓存
    DWORD dwRc = SaveTmpOfSubscribe(consumer, producer, event);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    /// 如果找不到生产者，只有等生产者创建时再处理
    IT_NOTIFY it = m_events.find(producer);
    if (it == m_events.end())
    {
        return SUCCESS;
    }

    /// 找到生产者，进行事件订阅
    CNotifyPool *pPool = &((*it).second);
    if (!pPool)
    {
        return FAILURE;
    }

    return pPool->OnSubscribe(event);
}

/*******************************************************
  函 数 名: CNotify::SaveTmpOfSubscribe
  描    述: 处理订阅的本地缓冲
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CNotify::SaveTmpOfSubscribe(DWORD consumer, DWORD producer, DWORD event)
{
    SubscribeNode tmpNode = {consumer, producer, event};

    /// 如果本地缓冲已经存在，则返回错误
    if (m_SubscribeTmp.Pos(m_SubscribeTmp.Find(&tmpNode)))
    {
        return FAILURE;
    }

    return m_SubscribeTmp.Append(&tmpNode);
}

/*******************************************************
  函 数 名: CNotify::ProcTmpOfSubscribe
  描    述: 处理订阅的临时缓存
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CNotify::ProcTmpOfSubscribe(CNotifyPool *pPool)
{
    if (!pPool) return;

    IObject *pProducer = pPool->GetProducer();
    if (!pProducer) return;

    DWORD dwID = pProducer->ID();
    SubscribeNode *pNode = (SubscribeNode *)(m_SubscribeTmp.Pos(m_SubscribeTmp.Find(&dwID, CompareSubscribeNode)));
    if (!pNode) return;

    (void)pPool->OnSubscribe(pNode->event);
}


/*******************************************************
  函 数 名: CNotify::CompareSubscribeNode
  描    述: 比较订阅节点
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool CNotify::CompareSubscribeNode(void *pNode, void *pKey)
{
    if (!pNode || !pKey) return false;

    SubscribeNode *pSubscribeNode = (SubscribeNode *)pNode;
    DWORD dwProducer = *(DWORD *)pKey;
    return (pSubscribeNode->producer == dwProducer)? true : false;
}

