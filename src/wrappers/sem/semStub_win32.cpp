/// -------------------------------------------------
/// semStub_win32.cpp : windows操作系统信号量实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "dcop.h"

#if DCOP_OS == DCOP_OS_WINDOWS

#include "semApi.h"
#include <windows.h>


/// =================================================
///     等待信号
/// -------------------------------------------------
/// WaitForSingleObject成功时有三个返回值:
/// WAIT_ABANDONED - 如果是mutex对象时，并且获取对象的
///     线程没有释放就结束了，这个时候对象的拥有权就会
///     给当前调用的线程，但是信号状态值是无
/// WAIT_OBJECT_0  - 获取对象的信号状态值是有
/// WAIT_TIMEOUT   - 等待时间到了，但是对象状态还是无
/// =================================================


/// =================================================
///     mutex和critical section的区别
/// -------------------------------------------------
/// 1.锁住一个未被拥有的mutex，比锁住一个未被拥有的
///   critical section需要花费几乎100倍的时间。因为
///   critical section不需要进入操作系统核心。
/// 2.mutex可以跨进程使用，critical section则只能够在
///   同一进程中使用。
/// 3.等待一个mutex时，你可以指定“结束等待”的时间长
///   度；而critical section 不需要指定等待时间，直到
///   进入互斥区为止。
/// =================================================


void STUB_CSInitialize(OSHANDLE *pHandle, 
                        const char *file, int line)
{
    if (!pHandle)
    {
        return;
    }

    *pHandle = NULL;

    #undef new
    CRITICAL_SECTION *pCS = new (file, line) CRITICAL_SECTION;
    #define new new(__FILE__, __LINE__)
    if (!pCS)
    {
        return;
    }

    InitializeCriticalSection(pCS);

    *pHandle = (OSHANDLE)(pCS);
}

void STUB_CSDelete(OSHANDLE Handle)
{
    if (!Handle)
    {
        return;
    }

    CRITICAL_SECTION *pCS = (CRITICAL_SECTION *)Handle;

    DeleteCriticalSection(pCS);

    delete pCS;
}

void STUB_CSEnter(OSHANDLE Handle)
{
    if (!Handle)
    {
        return;
    }

    EnterCriticalSection((CRITICAL_SECTION *)Handle);
}

void STUB_CSLeave(OSHANDLE Handle)
{
    if (!Handle)
    {
        return;
    }

    LeaveCriticalSection((CRITICAL_SECTION *)Handle);
}

DWORD STUB_EventCreate(OSHANDLE *pHandle, 
                        BOOL bHaveEvent, 
                        const char *file, int line)
{
    if (!pHandle)
    {
        return ERRCODE_SEM_WRONG_HANDLE;
    }

    *pHandle = (OSHANDLE)CreateEvent(
                    NULL,
                    FALSE,
                    bHaveEvent,
                    NULL);
    ERROR_CHECK((*pHandle));

    return SUCCESS;

ERROR_LABEL:

    DWORD dwRc = GetLastError();
    if (!dwRc)
    {
        dwRc = FAILURE;
    }

    return dwRc;
}

DWORD STUB_EventDestroy(OSHANDLE Handle)
{
    if (!Handle)
    {
        return ERRCODE_SEM_WRONG_HANDLE;
    }

    ERROR_CHECK(CloseHandle((HANDLE)Handle));

    return SUCCESS;

ERROR_LABEL:

    DWORD dwRc = GetLastError();
    if (!dwRc)
    {
        dwRc = FAILURE;
    }

    return dwRc;
}

DWORD STUB_EventSend(OSHANDLE Handle)
{
    if (!Handle)
    {
        return ERRCODE_SEM_WRONG_HANDLE;
    }

    ERROR_CHECK(SetEvent((HANDLE)Handle));

    return SUCCESS;

ERROR_LABEL:

    DWORD dwRc = GetLastError();
    if (!dwRc)
    {
        dwRc = FAILURE;
    }

    return dwRc;
}

DWORD STUB_EventRecv(OSHANDLE Handle, DWORD dwMilliseconds)
{
    if (!Handle)
    {
        return ERRCODE_SEM_WRONG_HANDLE;
    }

    ERROR_CHECK(WAIT_OBJECT_0 == WaitForSingleObject((HANDLE)Handle, 
        ((dwMilliseconds == OSWAIT_FOREVER)? INFINITE : dwMilliseconds)));

    return SUCCESS;

ERROR_LABEL:

    DWORD dwRc = GetLastError();
    if (!dwRc)
    {
        dwRc = FAILURE;
    }

    return dwRc;
}

DWORD STUB_SemCreate(OSHANDLE *pHandle, 
                        DWORD dwInitCount, 
                        DWORD dwMaxCount, 
                        const char *file, int line)
{
    if (!pHandle)
    {
        return ERRCODE_SEM_WRONG_HANDLE;
    }

    *pHandle = (OSHANDLE)CreateSemaphore(
                    NULL,
                    (LONG)dwInitCount,
                    (LONG)dwMaxCount,
                    NULL);
    ERROR_CHECK((*pHandle));

    return SUCCESS;

ERROR_LABEL:

    DWORD dwRc = GetLastError();
    if (!dwRc)
    {
        dwRc = FAILURE;
    }

    return dwRc;
}

DWORD STUB_SemDestroy(OSHANDLE Handle)
{
    if (!Handle)
    {
        return ERRCODE_SEM_WRONG_HANDLE;
    }

    ERROR_CHECK(CloseHandle((HANDLE)Handle));

    return SUCCESS;

ERROR_LABEL:

    DWORD dwRc = GetLastError();
    if (!dwRc)
    {
        dwRc = FAILURE;
    }

    return dwRc;
}

DWORD STUB_SemTake(OSHANDLE Handle, DWORD dwMilliseconds)
{
    if (!Handle)
    {
        return ERRCODE_SEM_WRONG_HANDLE;
    }

    ERROR_CHECK(WAIT_OBJECT_0 == WaitForSingleObject((HANDLE)Handle, 
        ((dwMilliseconds == OSWAIT_FOREVER)? INFINITE : dwMilliseconds)));

    return SUCCESS;

ERROR_LABEL:

    DWORD dwRc = GetLastError();
    if (!dwRc)
    {
        dwRc = FAILURE;
    }

    return dwRc;
}

DWORD STUB_SemGive(OSHANDLE Handle, DWORD dwGiveCount)
{
    if (!Handle)
    {
        return ERRCODE_SEM_WRONG_HANDLE;
    }

    ERROR_CHECK(ReleaseSemaphore((HANDLE)Handle, dwGiveCount, NULL));

    return SUCCESS;

ERROR_LABEL:

    DWORD dwRc = GetLastError();
    if (!dwRc)
    {
        dwRc = FAILURE;
    }

    return dwRc;
}

void vRegOsSemStubFunc()
{
    LockFuncs lockFuncs = 
    {
        STUB_CSInitialize,
        STUB_CSDelete,
        STUB_CSEnter,
        STUB_CSLeave
    };

    vSetLockFuncs(&lockFuncs);

    EventFuncs eventFuncs = 
    {
        STUB_EventCreate,
        STUB_EventDestroy,
        STUB_EventSend,
        STUB_EventRecv
    };

    vSetEventFuncs(&eventFuncs);

    CounterFuncs counterFuncs = 
    {
        STUB_SemCreate,
        STUB_SemDestroy,
        STUB_SemTake,
        STUB_SemGive
    };

    vSetCounterFuncs(&counterFuncs);
}

CPPBUILDUNIT_AUTO(vRegOsSemStubFunc, 0);


#endif

