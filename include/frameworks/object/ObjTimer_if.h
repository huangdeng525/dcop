/// -------------------------------------------------
/// ObjTimer_if.h : 定时器对象公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJTIMER_IF_H_
#define _OBJTIMER_IF_H_

#include "Object_if.h"
#include "string/dstring.h"


/////////////////////////////////////////////////////
///                    使用说明
/// -------------------------------------------------
/// 1. 定时器时间单位为毫秒(ms)，精度为1s和100ms两种:
///    30s(含30s)以上的超时时间，使用精度为1s；
///    30s以内的超时时间，使用精度为100ms。
/// 2. 无论是超时定时器还是循环定时器都需要主动停止，
///    否则就算已经超时，但是句柄没人释放。
/// 3. 另外如果需要在本地有超时队列的，
///    可以使用时间轮: IWheel
/////////////////////////////////////////////////////


/// 定义ITimer版本号
INTF_VER(ITimer, 1, 0, 0);


/// 定时器接口
interface ITimer : public IObject
{
    /////////////////////////////////////////////////
    /// 中心定时器接口
    /// ---------------------------------------------
    /// 由定时器对象自己提供的唯一一个公用的定时器队列
    /////////////////////////////////////////////////

    /// 定时器类型
    enum TYPE
    {
        TYPE_LOOP = 0,                              // 循环定时器
        TYPE_NOLOOP                                 // 超时定时器
    };

    /// 定时器句柄
    interface IWheel;
    typedef struct Node
    {
        IWheel *                m_wheel;            // 时间轮对象
        Node *                  m_timer;            // 定时器句柄
        DWORD                   m_index;            // 槽位索引值
        DWORD                   m_tbase;            // 时间基础值
    } * Handle;                                     // 定时器句柄

    /// 启动一个定时器(返回NULL为失败，否则为成功)
    virtual Handle Start(
                        TYPE timerType,             // 定时器类型
                        DWORD dwEvent,              // 定时器事件
                        DWORD dwTimeOut,            // 定时器超时时间
                        IObject *recver             // 定时器接收对象
                        ) = 0;

    /// 停止一个定时器
    virtual void Stop(
                        Handle hTimer               // 定时器句柄
                        ) = 0;


    /////////////////////////////////////////////////
    /// 时间轮队列接口
    /// ---------------------------------------------
    /// 各个对象可以使用这些接口来创建自己的定时器队列
    /////////////////////////////////////////////////

    interface IWheel
    {
        /// 超时处理回调函数
        typedef void (*TIMEOUT_PROC)(ITimer::Handle handle, void *para);

        /// 创建时间轮实例
        static IWheel *CreateInstance(DWORD dwWheelID,
                        DWORD dwSlotCount,
                        IWheel *pHigherWheel,
                        IWheel *pLowerWheel,
                        DWORD dwHashBase,
                        TIMEOUT_PROC fnTimeoutProc,
                        void *pTimeoutPara);

        /// 分配定时器句柄(输入存放到定时器节点值的地址和长度)
        static ITimer::Handle Alloc(void *pValue, DWORD dwLen);

        /// 释放定时器句柄(停止后释放句柄)
        static void Free(ITimer::Handle handle);

        /// 设置定时器节点值(如果在Alloc中设置的不对，可用该接口重新设置)
        static void SetValue(ITimer::Handle handle, void *pValue, DWORD dwLen);

        /// 根据定时器句柄获取定时器节点值
        static void *GetValue(ITimer::Handle handle);

        /// 根据定时器句柄获取定时器轮对象
        static IWheel *GetWheel(ITimer::Handle handle);

        /// 设置时间基值到定时器句柄中(请参照test_timer.cpp中使用)
        static void SetTimeBase(ITimer::Handle handle, DWORD tbase);

        /// 获取时间基值
        static DWORD GetTimeBase(ITimer::Handle handle);

        /// 获取槽位索引值
        static DWORD GetIndex(ITimer::Handle handle);

        /// 获取循环圈数
        static DWORD GetCycle(ITimer::Handle handle);

        /// 获取实际句柄值(方便复制定时器信息)
        static ITimer::Handle GetHandle(ITimer::Handle handle);

        /// 获取显示字符串
        static void GetString(ITimer::Handle handle, CDString &str);

        /// 虚析构
        virtual ~IWheel();

        /// 定时器递增
        virtual void OnTick() = 0;

        /// 加入定时器句柄
        virtual DWORD Add(ITimer::Handle handle) = 0;

        /// 删除定时器句柄
        virtual void Del(ITimer::Handle handle) = 0;

        /// 获取定时器轮ID
        virtual DWORD GetWheelID() = 0;

        /// 后去HASH基值(请参照test_timer.cpp中使用)
        virtual DWORD GetHashBase() = 0;

        /// 获取刻度点(请参照test_timer.cpp中使用)
        virtual DWORD GetScalePoint() = 0;

        /// 获取槽位数
        virtual DWORD GetSlotCount() = 0;
    };

};


#endif // #ifndef _OBJTIMER_IF_H_

