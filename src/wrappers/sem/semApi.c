/// -------------------------------------------------
/// semApi.c : 操作系统sem实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "semApi.h"


LockFuncs       g_LockFuncs;
EventFuncs      g_EventFuncs;
CounterFuncs    g_CounterFuncs;


void vSetLockFuncs(const LockFuncs* pFuncs)
{
    if (pFuncs)
    {
        g_LockFuncs = *pFuncs;
    }
    else
    {
        (void)memset((void *)&g_LockFuncs, 0, sizeof(LockFuncs));
    }
}

void vSetEventFuncs(const EventFuncs* pFuncs)
{
    if (pFuncs)
    {
        g_EventFuncs = *pFuncs;
    }
    else
    {
        (void)memset((void *)&g_EventFuncs, 0, sizeof(EventFuncs));
    }
}

void vSetCounterFuncs(const CounterFuncs* pFuncs)
{
    if (pFuncs)
    {
        g_CounterFuncs = *pFuncs;
    }
    else
    {
        (void)memset((void *)&g_CounterFuncs, 0, sizeof(CounterFuncs));
    }
}

