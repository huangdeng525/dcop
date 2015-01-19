/// -------------------------------------------------
/// semType.h : 信号量定义私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "semType.h"
#include "semApi.h"


objLock *objLock::CreateInstance(const char *file, int line)
{
    #undef new
    return new (file, line) CLockBase(file, line);
    #define new new(__FILE__, __LINE__)
}

objLock::~objLock()
{
}

CLockBase::CLockBase(const char *file, int line)
{
    Initialize(file, line);
}

CLockBase::~CLockBase()
{
    Delete();
}

void CLockBase::Enter()
{
    OS_VDFUNC_CALL(Lock, Enter)(osBase::hGetHandle());
}

void CLockBase::Leave()
{
    OS_VDFUNC_CALL(Lock, Leave)(osBase::hGetHandle());
}

void CLockBase::Initialize(const char *file, int line)
{
    OSHANDLE hLock = 0;

    OS_VDFUNC_CALL(Lock, Initialize)(&hLock, file, line);

    osBase::vSetHandle(hLock);
}

void CLockBase::Delete()
{
    OS_VDFUNC_CALL(Lock, Delete)(osBase::hGetHandle());

    osBase::vSetHandle(0);
}

objEvent *objEvent::CreateInstance(bool bHaveEvent, const char *file, int line)
{
    #undef new
    return new (file, line) CEventBase(bHaveEvent, file, line);
    #define new new(__FILE__, __LINE__)
}

objEvent::~objEvent()
{
}

CEventBase::CEventBase(bool bHaveEvent, const char *file, int line)
{
    Create(bHaveEvent, file, line);
}

CEventBase::~CEventBase()
{
    (void)Destroy();
}

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

DWORD CEventBase::Send()
{
    return OS_FUNC_CALL(Event, Send)(osBase::hGetHandle());
}

DWORD CEventBase::Recv(DWORD waitMilliseconds)
{
    return OS_FUNC_CALL(Event, Recv)(osBase::hGetHandle(), waitMilliseconds);
}

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

objCounter *objCounter::CreateInstance(DWORD initCount, DWORD maxCount, const char *file, int line)
{
    #undef new
    return new (file, line) CCounterBase(initCount, maxCount, file, line);
    #define new new(__FILE__, __LINE__)
}

objCounter::~objCounter()
{
}

CCounterBase::CCounterBase(DWORD initCount, DWORD maxCount, const char *file, int line)
{
    Create(initCount, maxCount, file, line);
}

CCounterBase::~CCounterBase()
{
    (void)Destroy();
}

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

DWORD CCounterBase::Take(DWORD waitMilliseconds)
{
    return OS_FUNC_CALL(Counter, Take)(osBase::hGetHandle(), waitMilliseconds);
}

DWORD CCounterBase::Give(DWORD giveCount)
{
    return OS_FUNC_CALL(Counter, Give)(osBase::hGetHandle(), giveCount);
}

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

