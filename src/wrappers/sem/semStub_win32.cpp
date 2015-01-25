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


/*******************************************************
  函 数 名: STUB_CSInitialize
  描    述: 初始化互斥量
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
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

/*******************************************************
  函 数 名: STUB_CSDelete
  描    述: 删除互斥量
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
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

/*******************************************************
  函 数 名: STUB_CSEnter
  描    述: 进入互斥区
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void STUB_CSEnter(OSHANDLE Handle)
{
    if (!Handle)
    {
        return;
    }

    EnterCriticalSection((CRITICAL_SECTION *)Handle);
}

/*******************************************************
  函 数 名: STUB_CSLeave
  描    述: 离开互斥区
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void STUB_CSLeave(OSHANDLE Handle)
{
    if (!Handle)
    {
        return;
    }

    LeaveCriticalSection((CRITICAL_SECTION *)Handle);
}

/*******************************************************
  函 数 名: STUB_EventCreate
  描    述: 创建事件
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
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

/*******************************************************
  函 数 名: STUB_EventDestroy
  描    述: 删除事件
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
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

/*******************************************************
  函 数 名: STUB_EventSend
  描    述: 发送事件
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
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

/*******************************************************
  函 数 名: STUB_EventRecv
  描    述: 接收事件
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
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

/*******************************************************
  函 数 名: STUB_SemCreate
  描    述: 创建信号量
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
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

/*******************************************************
  函 数 名: STUB_SemDestroy
  描    述: 删除信号量
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
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

/*******************************************************
  函 数 名: STUB_SemTake
  描    述: 获取信号量
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
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

/*******************************************************
  函 数 名: STUB_SemGive
  描    述: 释放信号量
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
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

/*******************************************************
  函 数 名: vRegOsSemStubFunc
  描    述: 注册信号量桩
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
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

/*******************************************************
  函 数 名: CPPBUILDUNIT_AUTO
  描    述: 自动安装信号量桩
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CPPBUILDUNIT_AUTO(vRegOsSemStubFunc, 0);


#endif // #if DCOP_OS == DCOP_OS_WINDOWS

