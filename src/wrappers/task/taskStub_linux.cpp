/// -------------------------------------------------
/// taskStub_linux.cpp : linux操作系统任务实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "dcop.h"

#if DCOP_OS == DCOP_OS_LINUX

#include "taskApi.h"
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>


/// -------------------------------------------------
/// 实现任务接口桩
/// -------------------------------------------------

DWORD STUB_TaskCreate(OSHANDLE *pHandle,
            const char *szName,
            DWORD *pdwID,
            void (*pEntry)(void *pPara),
            DWORD dwStackSize,
            DWORD dwPriority,
            void *pPara)
{
    (void)dwPriority;
    /// linux系统默认是SCHED_OTHER调度策略，是不支持设置优先级的，以后可以使用其他策略支持

    if (!pHandle)
    {
        return ERRCODE_TASK_WRONG_HANDLE;
    }

    DWORD dwRc;
    pthread_t a_thread;
    pthread_attr_t thread_attr;
    /// void *thread_result;

    dwRc = pthread_attr_init(&thread_attr);
    if (dwRc != 0)
    {
        return (errno)? (DWORD)(errno) : ERRCODE_TASK_CREATE_FAIL;
    }

    if (dwStackSize)
    {
        (void)pthread_attr_setstacksize(&thread_attr, dwStackSize);
    }

    dwRc = (DWORD)pthread_create(&a_thread, &thread_attr, (void* (*)(void*))pEntry, pPara);
    (void)pthread_attr_destroy(&thread_attr);
    if (dwRc != 0)
    {
        return (errno)? (DWORD)(errno) : ERRCODE_TASK_CREATE_FAIL;
    }

    *pHandle = (OSHANDLE)a_thread;
    return SUCCESS;
}

DWORD STUB_TaskDestroy(OSHANDLE Handle, 
        const char *szName,
        DWORD dwID)
{
    pthread_t a_thread = (pthread_t)Handle;

    if (pthread_self() == a_thread)
    {
        /// 参见taskStub_Win32中的注释，退出任务主要就是任务自己退出
        /// pthread_exit(NULL);
    }
    else
    {
        (void)pthread_cancel(a_thread);
    }

    return SUCCESS;
}

void STUB_TaskDelay(DWORD dwMilliseconds)
{
    /// 在linux下调用sleep是用时钟的,一个进程的时钟系统是有限制的
    /// 如果每个线程使用sleep,到了最大的数量,最终进程会挂起.最好是用select取代
    /// usleep和nanosleep不太精确.sleep可能收到系统信号提前返回
    /// select是使用tick轮询方式的(可能效率不高,精度单位是tick的单位:一般10ms)

    struct timeval timeout;

    timeout.tv_sec = dwMilliseconds / THOUSAND;
    timeout.tv_usec = (dwMilliseconds % THOUSAND) * THOUSAND;

    (void)select(0, NULL, NULL, NULL, &timeout);
}

DWORD STUB_TaskCurrent()
{
    return (DWORD)syscall(SYS_gettid);
}

void vRegOsTaskStubFunc()
{
    TaskFuncs funcs = 
    {
        STUB_TaskCreate,
        STUB_TaskDestroy,
        STUB_TaskDelay,
        STUB_TaskCurrent
    };

    vSetTaskFuncs(&funcs);
}

CPPBUILDUNIT_AUTO(vRegOsTaskStubFunc, 0);


/// -------------------------------------------------
/// 实现原子操作
/// -------------------------------------------------

#include "task.h"

objAtomic::T objAtomic::Inc(T &val)
{
    return (T)__sync_add_and_fetch(&val, 1);
}

objAtomic::T objAtomic::Dec(T &val)
{
    return (T)__sync_sub_and_fetch(&val, 1);
}

objAtomic::T objAtomic::Add(T &val, T add)
{
    return (T)__sync_add_and_fetch(&val, add);
}

objAtomic::T objAtomic::Sub(T &val, T sub)
{
    return (T)__sync_sub_and_fetch(&val, sub);
}

objAtomic::T objAtomic::Set(T &val, T set)
{
    return (T)__sync_lock_test_and_set(&val, set);
}

bool objAtomic::CAS(T &val, T cmp, T swap)
{
    return __sync_bool_compare_and_swap(&val, cmp, swap);
}


/// -------------------------------------------------
/// 实现随机数操作
/// -------------------------------------------------

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

class CDevRandom : public objRandom
{
public:
    static const DWORD SRAND_COUNT = 32;
    static const DWORD TRY_COUNT = 16;

public:
    CDevRandom();
    ~CDevRandom();

    void Gen(void *pBuf, DWORD dwLen);

private:
    void Rand(void *pBuf, DWORD dwLen);
};

CDevRandom::CDevRandom()
{
    unsigned int ticks;
    struct timeval tv;
    int fd;
    gettimeofday(&tv, NULL);
    ticks = tv.tv_sec + tv.tv_usec;
    fd = open("/dev/urandom", O_RDONLY);
    if (fd >= 0) // linux句柄有可能是0，失败时返回(-1)
    {
        unsigned int r;
        for (DWORD i = 0; i < SRAND_COUNT; i++)
        {
            (void)read(fd, &r, sizeof(r));
            ticks += r;
        }

        close(fd);
    }

    srand(ticks);
}

CDevRandom::~CDevRandom()
{
}

void CDevRandom::Gen(void *pBuf, DWORD dwLen)
{
    if (!pBuf || !dwLen)
    {
        return;
    }

    DWORD dwReadLen = 0;
    
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd >= 0)
    {
        DWORD dwTryTimes = 0;
        while ((dwTryTimes < TRY_COUNT) && (dwReadLen < dwLen))
        {
            int r = read(fd, (BYTE *)pBuf + dwReadLen, dwLen - dwReadLen);
            if (r > 0) dwReadLen += (DWORD)r;
            dwTryTimes++;
        }

        close(fd);
    }

    if (dwReadLen < dwLen)
    {
        Rand((BYTE *)pBuf + dwReadLen, dwLen - dwReadLen);
    }
}

void CDevRandom::Rand(void *pBuf, DWORD dwLen)
{
    DWORD dwReadLen = 0;

    while (dwReadLen < dwLen)
    {
        *((BYTE *)pBuf + dwReadLen) = (BYTE)(rand() % 256);
        dwReadLen++;
    }
}

objRandom *objRandom::CreateInstance(const char *file, int line)
{
    #undef new
    return new (file, line) CDevRandom();
    #define new new(__FILE__, __LINE__)
}

objRandom::~objRandom()
{
}


/// -------------------------------------------------
/// 实现获取调用栈操作
/// -------------------------------------------------

#include <execinfo.h>

#define CALLSTACK_DEPTH_MAX 10

void ShowCallStack(LOG_PRINT print, LOG_PARA para, int depth)
{
    void *stack_addr[CALLSTACK_DEPTH_MAX];
    int layer = 0;
    char **ppstack_funcs = 0;

    if (!print)
    {
        print = PrintToConsole;
        para = 0;
    }

    layer = backtrace(stack_addr, ARRAY_SIZE(stack_addr));
    ppstack_funcs = backtrace_symbols(stack_addr, layer);
    if (!ppstack_funcs)
    {
        print("Obtained CallStack Failed! \r\n", para);
        free(ppstack_funcs);
        return;
    }

    if (depth && (layer > (depth + 1)))
    {
        layer = depth + 1;
    }

    print(STR_FORMAT("Obtained CallStack %d Frames! \r\n", (layer - 1)), para);
    for(int i = 1; i < layer; i++)
    {
        print(STR_FORMAT("%s \r\n", ppstack_funcs[i]), para);
    }

    free(ppstack_funcs);
}


#endif

