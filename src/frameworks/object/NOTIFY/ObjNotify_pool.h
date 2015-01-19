/// -------------------------------------------------
/// ObjNotify_pool.h : 通知事件缓冲池公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJNOTIFY_POOL_H_
#define _OBJNOTIFY_POOL_H_

#include "ObjNotify_if.h"


/// -------------------------------------------------
/// 事件缓冲池
/// -------------------------------------------------
class CNotifyPool : public INotify::IPool
{
public:
    struct Node
    {        
        DWORD m_dwEvent;                        // 事件属性值
        DWORD m_dwSubscribedCount;              // 被订阅次数
        DWORD m_dwProducedCount;                // 被产生次数
        DWORD m_dwDispatchedCount;              // 被分发次数
    };

public:
    CNotifyPool();
    ~CNotifyPool();

    DWORD Init(IObject *owner, DWORD *events, DWORD count, INotify::GET_MSG_EVENT fnGetMsgEvent);

    IObject *GetProducer() {return m_pOwner;}

    DWORD OnSubscribe(DWORD event);

    DWORD OnPublish(DWORD event);

    bool IsSubscribedMsgEvent(objMsg *msg, DWORD event);

    void HaveDispatchedMsgEvent(DWORD event);

    void Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv);

private:
    IObject *m_pOwner;                          // 事件生产者
    Node *m_pEventsBuf;                         // 事件缓冲区
    DWORD m_dwEventCount;                       // 事件数量值
    INotify::GET_MSG_EVENT m_fnGetMsgEvent;     // 事件值获取
};



#endif // #define _OBJNOTIFY_POOL_H_

