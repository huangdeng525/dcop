/// -------------------------------------------------
/// ObjStatus_if.h : 状态机对象公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJSTATUS_IF_H_
#define _OBJSTATUS_IF_H_

#include "Object_if.h"


/////////////////////////////////////////////////////
///                    使用说明
/// -------------------------------------------------
/// 1.状态机为子对象，必须被所有的对象单独创建
/// 2.构造时所有者对象将自己作为状态机的父对象传入
/// 2.状态机按照注册顺序，状态值从0开始递增
/// 3.状态机可以按照默认顺序递增，也可以手动设置迁移
/// 4.状态机为被动状态机，驱动和回馈都以对象事件进行:
///   ======== [驱动] ================ [状态机] ========== [回馈事件] ============== [对象] ======
///   User(对象事件处理后) ----------> [Start]    ---> DCOP_MSG_STATUS_START   ---> User(对象事件)
///                                     |
///                                     V
///   User(对象事件处理后) ----> +---> [NewState] ---> DCOP_MSG_STATUS_NEW     ---> User(对象事件)
///                              |      |
///                        (循环)|      V (超时)
///   User(对象定时器事件) ----> +------+----->   ---> DCOP_MSG_STATUS_TIMEOUT ---> User(对象事件)
///                                     |
///                                     V (结束)
///   User(对象事件处理后) ----------> [Stop]     ---> DCOP_MSG_STATUS_STOP    ---> User(对象事件)
/////////////////////////////////////////////////////


/// 定义IStatusMachine版本号
INTF_VER(IStatus, 1, 0, 0);


/// 状态机接口
interface IStatus : public IObject
{   
    /// 状态属性(注册)
    struct REG
    {
        const char *m_cszName;              // 状态名(状态机里只保存常量字符串地址)
        DWORD m_dwNextStateDefault;         // 默认下一状态(状态值)
        DWORD m_dwTimeout;                  // 状态超时时间(以Tick()接口被调用的间隔为单位)
        DWORD m_dwBackStateWhenTimeout;     // 超时后回退的状态
    };

    /// 状态属性(运行时)
    struct STATE : public REG
    {
        DWORD m_dwRunStateValue;            // 运行时状态值(从0开始，也是注册顺序的索引)
        DWORD m_dwRunPrevState;             // 运行时前一状态
        DWORD m_dwRunTimeValue;             // 运行时时间计时刻度
    };

    /// 无效状态值
    static const DWORD Invalid = (DWORD)(-1);

    /// 注册状态(返回0为成功，否则为失败)
    virtual DWORD Reg(
                REG *pStateNodes,           // 状态节点列表
                DWORD dwStateCount          // 状态数
                ) = 0;

    /// 开始
    virtual DWORD Start() = 0;

    /// 停止
    virtual DWORD Stop() = 0;

    /// 定时计数
    virtual DWORD Tick() = 0;

    /// 迁移到新状态(默认为预设状态)(返回0为成功，否则为失败)
    virtual DWORD SetNewState(DWORD dwNewState = Invalid) = 0;

    /// 获取当前状态值(返回Invalid为无效值，否则为有效值)
    virtual DWORD GetCurState() = 0;

    /// 获取指定状态的信息
    virtual STATE *GetStateInfo(DWORD dwState) = 0;

    /// 是否是运行状态
    virtual bool bRun() = 0;
};


#endif // #ifndef _OBJSTATUSMACHINE_IF_H_

