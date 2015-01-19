/// -------------------------------------------------
/// ObjStatus_main.h : 状态机管理私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJSTATUS_MAIN_H_
#define _OBJSTATUS_MAIN_H_

#include "ObjStatus_if.h"


class CStatus : public IStatus
{
public:
    CStatus(Instance *piParent, int argc, char **argv);
    ~CStatus();

    DCOP_DECLARE_INSTANCE;
    DCOP_DECLARE_IOBJECT;

    DWORD Reg(REG *pStateNodes, DWORD dwStateCount);

    DWORD Start();

    DWORD Stop();

    DWORD Tick();

    DWORD SetNewState(DWORD dwNewState = IStatus::Invalid);

    DWORD GetCurState();

    STATE *GetStateInfo(DWORD dwState);

    bool bRun()
    {
        AutoObjLock(this);
        return m_bRunning;
    }

private:
    DWORD EventToOwner(DWORD event);

private:
    STATE *m_pStateNodes;                   // 状态节点列表
    DWORD m_dwStateCount;                   // 状态个数
    DWORD m_dwCurState;                     // 当前状态值
    bool m_bRunning;                        // 是否正在运行
};


#endif // #ifndef _STATUS_MAIN_H_

