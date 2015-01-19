/// -------------------------------------------------
/// ObjResponse_pool.h : 响应缓冲池私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJRESPONSE_POOL_H_
#define _OBJRESPONSE_POOL_H_

#include "ObjResponse_if.h"
#include "ObjDispatch_if.h"
#include "ObjTimer_if.h"


/// -------------------------------------------------
/// 命令响应缓冲池
/// -------------------------------------------------
class CResponsePool : public IResponse::IPool
{
public:
    /// 缓冲池节点
    struct Node
    {
        WORD m_wNewIdx;                     // 新的索引
        WORD m_wOldIdx;                     // 老的索引
        DCOP_SESSION_HEAD *m_pSession;      // 会话消息
        DWORD m_dwMsgType;                  // 消息类型
        DWORD m_dwSrcID;                    // 源ID
        DWORD m_dwDstID;                    // 目的ID
        DWORD m_dwRspMsgType;               // 响应消息类型
        ITimer::Handle m_hTimer;            // 定时器句柄
    };

    /// 定时器轮索引、ID、基值和槽位数量
    static const DWORD WHEEL_S_SEC = 0;
    static const DWORD WHEEL_S_SEC_ID = 1;
    static const DWORD WHEEL_S_HASH_BASE = 1000;
    static const DWORD WHEEL_S_SEC_SLOT_COUNT = 32;

    /// TimerValue是注册的定时器轮节点值
    struct TimerValue
    {
        WORD m_wNewIdx;                     // 新的索引
        WORD m_wOldIdx;                     // 老的索引
        DWORD m_dwTimeout;                  // 超时时间
        DWORD m_dwSendTryTimes;             // 重传次数
    };

public:
    CResponsePool();
    ~CResponsePool();

    DWORD Init(IObject *root, IObject *owner, DWORD count);

    DWORD OnReq(DCOP_SESSION_HEAD *pSession,
                        DWORD dwMsgType,
                        DWORD dwSrcID,
                        DWORD dwDstID,
                        DWORD dwRspMsgType,
                        DWORD dwTimeout,
                        DWORD dwSendTryTimes);

    DWORD OnRsp(DCOP_SESSION_HEAD *pSession);

    void OnTick();

private:
    Node *GetNode(DWORD dwIdx);
    DWORD GenIdleIdx();
    static void OnTimeout(ITimer::Handle handle, void *para);
    DWORD InsertToWheel(ITimer::Handle hTimer, DWORD dwTimeOut = 0);
    void  DelFromWheel(ITimer::Handle hTimer);
    DWORD TrySendTimes(TimerValue *pValue, ITimer::Handle hTimer);
    void OnRspTimeout(Node *pNode);

private:
    IObject *m_pOwner;
    Node *m_pNodeBuf;
    DWORD m_dwNodeCount;
    DWORD m_dwLastIdx;
    IDispatch *m_piDispatch;
    ITimer::IWheel *m_pTimerWheel;
};



#endif // #ifndef _OBJRESPONSE_POOL_H_

