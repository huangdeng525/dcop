/// -------------------------------------------------
/// semStub_linux.cpp : linux操作系统信号量实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "task.h"

#if DCOP_OS == DCOP_OS_LINUX

#include "semApi.h"
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <errno.h>


#define  LINUX_SEM_DELAY_TICK 50

/*******************************************************
  函 数 名: STUB_MutexInitialize
  描    述: 初始化互斥量
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void STUB_MutexInitialize(OSHANDLE *pHandle, 
                        const char *file, int line)
{
    if (!pHandle)
    {
        return;
    }

    *pHandle = NULL;

    /// 申请句柄空间
    #undef new
    pthread_mutex_t *pMutex = new (file, line) pthread_mutex_t;
    #define new new(__FILE__, __LINE__)
    if (!pMutex)
    {
        return;
    }

    /// 设置同一线程嵌套锁属性
    pthread_mutexattr_t attrMutex;
    pthread_mutexattr_init(&attrMutex);
    pthread_mutexattr_settype(&attrMutex, PTHREAD_MUTEX_RECURSIVE_NP);

    /// 初始化互斥锁
    (void)pthread_mutex_init(pMutex, &attrMutex);

    *pHandle = (OSHANDLE)(pMutex);
}

/*******************************************************
  函 数 名: STUB_MutexDestroy
  描    述: 删除互斥量
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void STUB_MutexDestroy(OSHANDLE Handle)
{
    if (!Handle)
    {
        return;
    }

    pthread_mutex_t *pMutex = (pthread_mutex_t *)Handle;
    (void)pthread_mutex_destroy(pMutex);

    delete pMutex;
}

/*******************************************************
  函 数 名: STUB_MutexEnter
  描    述: 进入互斥区
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void STUB_MutexEnter(OSHANDLE Handle)
{
    if (!Handle)
    {
        return;
    }

    (void)pthread_mutex_lock((pthread_mutex_t *)Handle);
}

/*******************************************************
  函 数 名: STUB_MutexLeave
  描    述: 离开互斥区
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void STUB_MutexLeave(OSHANDLE Handle)
{
    if (!Handle)
    {
        return;
    }

    (void)pthread_mutex_unlock((pthread_mutex_t *)Handle);
}

/*******************************************************
  函 数 名: STUB_CondInitialize
  描    述: 初始化条件信号量
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD STUB_CondInitialize(OSHANDLE *pHandle, 
                        BOOL bHaveEvent, 
                        const char *file, int line)
{
    if (!pHandle)
    {
        return ERRCODE_SEM_WRONG_HANDLE;
    }

    *pHandle = NULL;

    /// 申请句柄空间(互斥锁 + 条件变量)
    #undef new
    char *pTmp = new (file, line) char [sizeof(pthread_mutex_t) + sizeof(pthread_cond_t)];
    #define new new(__FILE__, __LINE__)
    if (!pTmp)
    {
        return ERRCODE_SEM_WRONG_HANDLE;
    }

    (void)memset(pTmp, 0, sizeof(pthread_mutex_t) + sizeof(pthread_cond_t));

    pthread_mutex_t *pMutex = (pthread_mutex_t *)pTmp;
    pthread_cond_t *pCond = (pthread_cond_t *)(pTmp + sizeof(pthread_mutex_t));

    /// 初始化互斥锁和条件变量
    (void)pthread_mutex_init(pMutex, NULL);
    (void)pthread_cond_init(pCond, NULL);

    if (bHaveEvent)
    {
        pthread_cond_signal(pCond);
    }

    *pHandle = (OSHANDLE)(pTmp);

    return SUCCESS;
}

/*******************************************************
  函 数 名: STUB_CondDestroy
  描    述: 删除条件信号量
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD STUB_CondDestroy(OSHANDLE Handle)
{
    if (!Handle)
    {
        return ERRCODE_SEM_WRONG_HANDLE;
    }

    char *pTmp = (char *)Handle;
    pthread_mutex_t *pMutex = (pthread_mutex_t *)pTmp;
    pthread_cond_t *pCond = (pthread_cond_t *)(pTmp + sizeof(pthread_mutex_t));

    (void)pthread_cond_destroy(pCond);
    (void)pthread_mutex_destroy(pMutex);

    delete [] pTmp;

    return SUCCESS;
}

/*******************************************************
  函 数 名: STUB_CondSend
  描    述: 发送条件信号量
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD STUB_CondSend(OSHANDLE Handle)
{
    if (!Handle)
    {
        return ERRCODE_SEM_WRONG_HANDLE;
    }

    pthread_mutex_t *pMutex = (pthread_mutex_t *)Handle;
    pthread_cond_t *pCond = (pthread_cond_t *)(pMutex + 1);

    (void)pthread_cond_signal(pCond);

    return SUCCESS;
}

/*******************************************************
  函 数 名: STUB_CondRecv
  描    述: 接收条件信号量
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD STUB_CondRecv(OSHANDLE Handle, DWORD dwMilliseconds)
{
    if (!Handle)
    {
        return ERRCODE_SEM_WRONG_HANDLE;
    }

    struct timespec abstime;
    struct timeval now;

    /// 获取当前时间
    gettimeofday(&now, NULL);
    abstime.tv_sec = now.tv_sec + dwMilliseconds / THOUSAND;
    abstime.tv_nsec = now.tv_usec + (dwMilliseconds % THOUSAND) * THOUSAND;

    /// 防止微秒溢出
    if (abstime.tv_nsec >= BILLION)
    {
        abstime.tv_sec += abstime.tv_nsec / BILLION;
        abstime.tv_nsec %= BILLION;
    }

    pthread_mutex_t *pMutex = (pthread_mutex_t *)Handle;
    pthread_cond_t *pCond = (pthread_cond_t *)(pMutex + 1);

    int iRc;
    pthread_mutex_lock(pMutex);
    if (OSWAIT_FOREVER == dwMilliseconds)
    {
        iRc = pthread_cond_wait(pCond, pMutex);
    }
    else
    {
        iRc = pthread_cond_timedwait(pCond, pMutex, &abstime);
    }
    pthread_mutex_unlock(pMutex);

    if ((iRc < 0) && (errno == ETIMEDOUT))
    {
        return ERRCODE_SEM_WAIT_TIMEOUT;
    }

    if (!iRc)
    {
        return SUCCESS;
    }

    return FAILURE;
}

/*******************************************************
  函 数 名: STUB_SemInitialize
  描    述: 初始化信号量
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD STUB_SemInitialize(OSHANDLE *pHandle, 
                        DWORD dwInitCount, 
                        DWORD dwMaxCount, 
                        const char *file, int line)
{
    if (!pHandle)
    {
        return ERRCODE_SEM_WRONG_HANDLE;
    }

    *pHandle = NULL;

    /// 申请句柄空间(信号灯 + 计数器)
    #undef new
    char *pTmp = new (file, line) char [sizeof(sem_t) + sizeof(DWORD)];
    #define new new(__FILE__, __LINE__)
    if (!pTmp)
    {
        return ERRCODE_SEM_WRONG_HANDLE;
    }

    (void)memset(pTmp, 0, sizeof(sem_t) + sizeof(DWORD));

    sem_t *pSem = (sem_t *)pTmp;
    DWORD *pdwCount = (DWORD *)(pTmp + sizeof(sem_t));

    /// 初始化信号灯
    (void)sem_init(pSem, 0, (unsigned int)dwInitCount);
    *pdwCount = dwMaxCount;

    *pHandle = (OSHANDLE)(pTmp);

    return SUCCESS;
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

    char *pTmp = (char *)Handle;
    sem_t *pSem = (sem_t *)pTmp;

    (void)sem_destroy(pSem);

    delete [] pTmp;

    return SUCCESS;
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

    char *pTmp = (char *)Handle;
    sem_t *pSem = (sem_t *)pTmp;

    /// 永远等待: 直接获取
    if (OSWAIT_FOREVER == dwMilliseconds)
    {
        if ((sem_wait(pSem) < 0) && (errno == ETIMEDOUT))
        {
            return ERRCODE_SEM_WAIT_TIMEOUT;
        }

        return SUCCESS;
    }

    /// 有限等待: linux信号量没有按照时间等待，只有延时后尝试获取
    DWORD timeout = 0;
    DWORD delayCount = dwMilliseconds / LINUX_SEM_DELAY_TICK;
    do
    {
        int iRc = sem_trywait(pSem);
        if (!iRc)
        {
            return SUCCESS;
        }

        if (errno != EPERM)
        {
            return (DWORD)errno;
        }

        objTask::Delay(LINUX_SEM_DELAY_TICK);
        timeout++;
    }while (timeout < delayCount);

    return ERRCODE_SEM_WAIT_TIMEOUT;
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

    char *pTmp = (char *)Handle;
    sem_t *pSem = (sem_t *)pTmp;
    DWORD *pdwCount = (DWORD *)(pTmp + sizeof(sem_t));

    int sval = 0;
    (void)sem_getvalue(pSem, &sval);
    if ((DWORD)sval >= (*pdwCount))
    {
        return ERRCODE_SEM_OVER_MAXCOUNT;
    }

    (void)sem_post(pSem);

    return SUCCESS;
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
        STUB_MutexInitialize,
        STUB_MutexDestroy,
        STUB_MutexEnter,
        STUB_MutexLeave
    };

    vSetLockFuncs(&lockFuncs);

    EventFuncs eventFuncs = 
    {
        STUB_CondInitialize,
        STUB_CondDestroy,
        STUB_CondSend,
        STUB_CondRecv
    };

    vSetEventFuncs(&eventFuncs);

    CounterFuncs counterFuncs = 
    {
        STUB_SemInitialize,
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


#endif // #if DCOP_OS == DCOP_OS_LINUX

