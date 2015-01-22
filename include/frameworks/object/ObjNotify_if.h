/// -------------------------------------------------
/// ObjNotify_if.h : 订阅发布器对象公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJNOTIFY_IF_H_
#define _OBJNOTIFY_IF_H_

#include "Object_if.h"


/////////////////////////////////////////////////////
///                  '事件ID'说明
/// -------------------------------------------------
/// 如果事件是对象属性事件(DCOP_MSG_OBJECT_EVENT):
///     '事件ID'等于消息会话中的'属性ID'
/// 如果事件是其他类型的事件(以对象实际事件为准):
///     '事件ID'等于消息类型值(比如定时器事件)
/////////////////////////////////////////////////////


/// -------------------------------------------------
/// 定义INotify版本号
/// -------------------------------------------------
INTF_VER(INotify, 1, 0, 0);


/// -------------------------------------------------
/// 对象通知接口(生产者发布事件、消费者订阅事件)
/// -------------------------------------------------
interface INotify : public IObject
{
    /// 从消息中获取事件值的回调函数
    typedef DWORD (*GET_MSG_EVENT)(objMsg *msg);
    
    /// 事件缓冲池(供生产者使用，在本地缓冲哪些事件被订阅)
    interface IPool
    {
        /// 订阅时
        virtual DWORD OnSubscribe(
                        DWORD event
                        ) = 0;

        /// 发布时
        virtual DWORD OnPublish(
                        DWORD event
                        ) = 0;
    };

    /// 创建缓冲池(供生产者调用)(请在Init接口调用)
    virtual IPool *CreatePool(
                        IObject *producer,          // 生产者
                        DWORD *events,              // 生产者事件列表
                        DWORD count,                // 事件个数
                        GET_MSG_EVENT fnGetMsgEvent = 0 // 消息事件值获取回调
                        ) = 0;

    /// 删除缓冲池(供生产者调用)(请在Fini接口调用)
    virtual void DestroyPool(
                        IPool *pool                 // 创建的缓冲池
                        ) = 0;

    /// 事件订阅(供消费者调用)(请在Init接口调用)
    virtual DWORD Subscribe(
                        DWORD consumer,             // 消费者
                        DWORD producer,             // 生产者
                        DWORD event                 // 事件
                        ) = 0;

};


/// -------------------------------------------------
/// 订阅事件 - 表首
/// -------------------------------------------------
#define SUBSCRIBE_EVENT_START(Notify)               \
    DWORD __rcSubscribe = SUCCESS;                  \
    INotify *__objNotify = Notify;                  \
    do {


/// -------------------------------------------------
/// 订阅事件 - 表项
/// -------------------------------------------------
#define SUBSCRIBE_EVENT_ITEM(Object, Event)         \
    __rcSubscribe = __objNotify->Subscribe(ID(), Object, Event); \
    if (__rcSubscribe != SUCCESS) break;


/// -------------------------------------------------
/// 订阅事件 - 表尾
/// -------------------------------------------------
#define SUBSCRIBE_EVENT_END                         \
    } while (0);                                    \
    if (__rcSubscribe != SUCCESS) return __rcSubscribe;


#endif // #ifndef _OBJNOTIFY_IF_H_

