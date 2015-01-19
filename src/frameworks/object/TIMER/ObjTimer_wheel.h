/// -------------------------------------------------
/// ObjTimer_wheel.h : 定时器的时间轮私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJTIMER_WHEEL_H_
#define _OBJTIMER_WHEEL_H_


/////////////////////////////////////////////////////
///                    时间轮
/// -------------------------------------------------
/// https://www.ibm.com/developerworks/cn/linux/l-cn-timers
/// 时间轮 (Timing-Wheel) 算法类似于一以恒定速度旋转的
/// 左轮手枪，枪的撞针则撞击枪膛，如果枪膛中有子弹，则
/// 会被击发；与之相对应的是：对于 PerTickBookkeeping，
/// 其最本质的工作在于以 Tick 为单位增加时钟，如果发现
/// 有任何定时器到期，则调用相应的 ExpiryProcessing 。
/// 设定一个循环为 N 个 Tick 单元，当前时间是在 S 个循
/// 环之后指向元素 i (i>=0 and i<= N - 1)，则当前时间 
/// (Current Time)Tc 可以表示为：Tc = S*N + i ；如果此
/// 时插入一个时间间隔 (Time Interval) 为 Ti 的定时器，
/// 设定它将会放入元素 n(Next) 中，则 n = （Tc + Ti）
/// mod N = (S*N + i + Ti) mod N = (i + Ti) mod N 。
/// 如果我们的 N 足够的大，显然 StartTimer，StopTimer，
/// PerTickBookkeeping 时，算法复杂度分别为 O(1)，O(1)，
/// O(1) 。
/// 如果需要支持的定时器范围非常的大，上面的实现方式则
/// 不能满足这样的需求。因为这样将消耗非常可观的内存，
/// 假设需要表示的定时器范围为：0 – 2^3-1ticks，则简
/// 单时间轮需要 2^32 个元素空间，这对于内存空间的使用
/// 将非常的庞大。
/// 水表中，为了表示度量范围，分成了不同的单位，比如 
/// 1000，100，10 等等，相似的，表示一个 32bits 的范围，
/// 也不需要 2^32 个元素的数组。
/////////////////////////////////////////////////////

#include "ObjTimer_if.h"
#include "list/dllist.h"


/// 定时器最小值
#define TIMER_VALUE_MIN         100

/// 定时器时间轮
class CTimerWheel : public ITimer::IWheel
{
public:
    /// TimerNode是定时器队列节点
    ///     添加到队列中时应该重新获取时间基值
    struct TimerNode : ITimer::Node
    {
        DLL_ENTRY(TimerNode)    m_field;            // 链表字段
        DWORD                   m_cycle;            // 循环圈数
        DWORD                   m_extra;            // 附加时间
    };
    /// 临时附加值m_extra的作用: 
    ///     附加值就是低级轮的当前位置的值，需要两次用到附加值:
    ///     上一轮刻度值(行进1~2格)，实际上多算了低级值的当前值(附加值)，
    ///     于是在下一轮后，再算一次保存的附加值，于是就变成这样:
    ///     "上一轮刻度值 - 附加值 + 附加值 + 超时时间 - 下一轮整圈值"
    ///     而 "上一轮刻度值" == "下一轮整圈值"，所以上面的公式就是:
    ///     "超时时间"自己。

    typedef DLL_HEAD(TimerNode) TimerSlot;          // 时间轮槽位

public:
    CTimerWheel();
    ~CTimerWheel();

    DWORD Init(DWORD dwWheelID,
            DWORD dwSlotCount,
            CTimerWheel *pHigherWheel,
            CTimerWheel *pLowerWheel,
            DWORD dwHashBase,
            TIMEOUT_PROC fnTimeoutProc,
            void *pTimeoutPara);

    void OnTick();

    DWORD Add(ITimer::Handle handle);
    void Del(ITimer::Handle handle);

    DWORD GetWheelID() {return m_dwWheelID;}
    DWORD GetHashBase() {return m_dwHashBase;}
    DWORD GetScalePoint() {return m_dwScalePoint;}
    DWORD GetSlotCount() {return m_dwSlotCount;}

private:
    void SlotTimeout(TimerSlot *pSlot);
    void HandleTimeout(ITimer::Handle handle);
    
private:
    CTimerWheel *           m_pHigherWheel;         // 高刻度轮子
    CTimerWheel *           m_pLowerWheel;          // 低刻度轮子
    DWORD                   m_dwWheelID;            // 时间轮ID
    DWORD                   m_dwHashBase;           // HASH基值
    TimerSlot *             m_pSlotTable;           // 时间轮槽位表
    DWORD                   m_dwSlotCount;          // 时间轮槽位个数
    DWORD                   m_dwScalePoint;         // 时间轮刻度指针
    TIMEOUT_PROC            m_fnTimeoutProc;        // 超时处理回调
    void *                  m_pTimeoutPara;         // 超时处理回调参数
};


#endif // #ifndef _OBJTIMER_WHEEL_H_

