/// -------------------------------------------------
/// semType.h : 信号量定义私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "semType.h"
#include "semApi.h"


/*******************************************************
  函 数 名: objLock::CreateInstance
  描    述: 创建互斥锁
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objLock *objLock::CreateInstance(const char *file, int line)
{
    #undef new
    return new (file, line) CLockBase(file, line);
    #define new new(__FILE__, __LINE__)
}

/*******************************************************
  函 数 名: objLock::~objLock
  描    述: 析构互斥锁
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objLock::~objLock()
{
}

/*******************************************************
  函 数 名: CLockBase::CLockBase
  描    述: CLockBase构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CLockBase::CLockBase(const char *file, int line)
{
    Initialize(file, line);
}

/*******************************************************
  函 数 名: CLockBase::~CLockBase
  描    述: CLockBase析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CLockBase::~CLockBase()
{
    Delete();
}

/*******************************************************
  函 数 名: CLockBase::Enter
  描    述: 进入互斥区
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLockBase::Enter()
{
    OS_VDFUNC_CALL(Lock, Enter)(osBase::hGetHandle());
}

/*******************************************************
  函 数 名: CLockBase::Leave
  描    述: 离开互斥区
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLockBase::Leave()
{
    OS_VDFUNC_CALL(Lock, Leave)(osBase::hGetHandle());
}

/*******************************************************
  函 数 名: CLockBase::Initialize
  描    述: 初始化互斥锁
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLockBase::Initialize(const char *file, int line)
{
    OSHANDLE hLock = 0;

    OS_VDFUNC_CALL(Lock, Initialize)(&hLock, file, line);

    osBase::vSetHandle(hLock);
}

/*******************************************************
  函 数 名: CLockBase::Delete
  描    述: 删除互斥锁
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLockBase::Delete()
{
    OS_VDFUNC_CALL(Lock, Delete)(osBase::hGetHandle());

    osBase::vSetHandle(0);
}

/*******************************************************
  函 数 名: objEvent::CreateInstance
  描    述: 创建事件
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objEvent *objEvent::CreateInstance(bool bHaveEvent, const char *file, int line)
{
    #undef new
    return new (file, line) CEventBase(bHaveEvent, file, line);
    #define new new(__FILE__, __LINE__)
}

/*******************************************************
  函 数 名: objEvent::~objEvent
  描    述: 析构事件
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objEvent::~objEvent()
{
}

/*******************************************************
  函 数 名: CEventBase::CEventBase
  描    述: CEventBase构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CEventBase::CEventBase(bool bHaveEvent, const char *file, int line)
{
    Create(bHaveEvent, file, line);
}

/*******************************************************
  函 数 名: CEventBase::~CEventBase
  描    述: CEventBase析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CEventBase::~CEventBase()
{
    (void)Destroy();
}

/*******************************************************
  函 数 名: CEventBase::Create
  描    述: 创建事件
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CEventBase::Create(bool bHaveEvent, const char *file, int line)
{
    OSHANDLE hEvent = 0;
    
    DWORD dwRc = OS_FUNC_CALL(Event, Create)(&hEvent, (bHaveEvent?TRUE:FALSE), file, line);
    if (dwRc)
    {
        return dwRc;
    }

    osBase::vSetHandle(hEvent);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CEventBase::Send
  描    述: 发送事件
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CEventBase::Send()
{
    return OS_FUNC_CALL(Event, Send)(osBase::hGetHandle());
}

/*******************************************************
  函 数 名: CEventBase::Recv
  描    述: 接收事件
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CEventBase::Recv(DWORD waitMilliseconds)
{
    return OS_FUNC_CALL(Event, Recv)(osBase::hGetHandle(), waitMilliseconds);
}

/*******************************************************
  函 数 名: CEventBase::Destroy
  描    述: 删除事件
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CEventBase::Destroy()
{
    DWORD dwRc = OS_FUNC_CALL(Event, Destroy)(osBase::hGetHandle());
    if (dwRc)
    {
        return dwRc;
    }

    osBase::vSetHandle(0);

    return SUCCESS;
}

/*******************************************************
  函 数 名: objCounter::CreateInstance
  描    述: 创建计数器
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objCounter *objCounter::CreateInstance(DWORD initCount, DWORD maxCount, const char *file, int line)
{
    #undef new
    return new (file, line) CCounterBase(initCount, maxCount, file, line);
    #define new new(__FILE__, __LINE__)
}

/*******************************************************
  函 数 名: objCounter::~objCounter
  描    述: 析构计数器
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objCounter::~objCounter()
{
}

/*******************************************************
  函 数 名: CCounterBase::CCounterBase
  描    述: CCounterBase构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CCounterBase::CCounterBase(DWORD initCount, DWORD maxCount, const char *file, int line)
{
    Create(initCount, maxCount, file, line);
}

/*******************************************************
  函 数 名: CCounterBase::~CCounterBase
  描    述: CCounterBase析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CCounterBase::~CCounterBase()
{
    (void)Destroy();
}

/*******************************************************
  函 数 名: CCounterBase::Create
  描    述: 创建计数器
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CCounterBase::Create(DWORD initCount, DWORD maxCount, const char *file, int line)
{
    OSHANDLE hCounter = 0;
    
    DWORD dwRc = OS_FUNC_CALL(Counter, Create)(&hCounter, initCount, maxCount, file, line);
    if (dwRc)
    {
        return dwRc;
    }

    osBase::vSetHandle(hCounter);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CCounterBase::Take
  描    述: 递增计数器
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CCounterBase::Take(DWORD waitMilliseconds)
{
    return OS_FUNC_CALL(Counter, Take)(osBase::hGetHandle(), waitMilliseconds);
}

/*******************************************************
  函 数 名: CCounterBase::Give
  描    述: 释放计数器
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CCounterBase::Give(DWORD giveCount)
{
    return OS_FUNC_CALL(Counter, Give)(osBase::hGetHandle(), giveCount);
}

/*******************************************************
  函 数 名: CCounterBase::Destroy
  描    述: 删除计数器
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CCounterBase::Destroy()
{
    DWORD dwRc = OS_FUNC_CALL(Counter, Destroy)(osBase::hGetHandle());
    if (dwRc)
    {
        return dwRc;
    }

    osBase::vSetHandle(0);

    return SUCCESS;
}

