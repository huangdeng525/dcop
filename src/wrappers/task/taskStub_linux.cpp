/// -------------------------------------------------
/// taskStub_linux.cpp : linux操作系统任务实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "dcop.h"

#if DCOP_OS == DCOP_OS_LINUX

/// -------------------------------------------------
/// 实现任务接口桩
/// -------------------------------------------------
#include "taskApi.h"
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

/*******************************************************
  函 数 名: STUB_TaskCreate
  描    述: 创建任务桩
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD STUB_TaskCreate(OSHANDLE *pHandle,
            const char *cszName,
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

    pthread_t a_thread;
    pthread_attr_t thread_attr;

    DWORD dwRc = pthread_attr_init(&thread_attr);
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

/*******************************************************
  函 数 名: STUB_TaskDestroy
  描    述: 删除任务桩
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD STUB_TaskDestroy(OSHANDLE Handle, 
        const char *cszName,
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

/*******************************************************
  函 数 名: STUB_TaskDelay
  描    述: 任务延时桩
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
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

/*******************************************************
  函 数 名: STUB_TaskCurrent
  描    述: 获取当前任务桩
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD STUB_TaskCurrent()
{
    return (DWORD)syscall(SYS_gettid);
}

/*******************************************************
  函 数 名: vRegOsTaskStubFunc
  描    述: 注册任务桩
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
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

/*******************************************************
  函 数 名: CPPBUILDUNIT_AUTO
  描    述: 自动安装任务桩
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CPPBUILDUNIT_AUTO(vRegOsTaskStubFunc, 0);


/// -------------------------------------------------
/// 实现原子操作
/// -------------------------------------------------
#include "task.h"

/*******************************************************
  函 数 名: objAtomic::Inc
  描    述: 原子递增
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objAtomic::T objAtomic::Inc(T &val)
{
    return (T)__sync_add_and_fetch(&val, 1);
}

/*******************************************************
  函 数 名: objAtomic::Dec
  描    述: 原子递减
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objAtomic::T objAtomic::Dec(T &val)
{
    return (T)__sync_sub_and_fetch(&val, 1);
}

/*******************************************************
  函 数 名: objAtomic::Add
  描    述: 原子加
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objAtomic::T objAtomic::Add(T &val, T add)
{
    return (T)__sync_add_and_fetch(&val, add);
}

/*******************************************************
  函 数 名: objAtomic::Sub
  描    述: 原子减
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objAtomic::T objAtomic::Sub(T &val, T sub)
{
    return (T)__sync_sub_and_fetch(&val, sub);
}

/*******************************************************
  函 数 名: objAtomic::Set
  描    述: 原子设置
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objAtomic::T objAtomic::Set(T &val, T set)
{
    return (T)__sync_lock_test_and_set(&val, set);
}

/*******************************************************
  函 数 名: objAtomic::CAS
  描    述: 原子比较交换锁
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
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

/// -------------------------------------------------
/// linux dev真随机数
/// -------------------------------------------------
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

/*******************************************************
  函 数 名: CDevRandom::CDevRandom
  描    述: CDevRandom构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
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

/*******************************************************
  函 数 名: CDevRandom::~CDevRandom
  描    述: CDevRandom析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDevRandom::~CDevRandom()
{
}

/*******************************************************
  函 数 名: CDevRandom::Gen
  描    述: 获取随机数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDevRandom::Gen(void *pBuf, DWORD dwLen)
{
    if (!pBuf || !dwLen)
    {
        return;
    }

    DWORD dwReadLen = 0;

    /// "/dev/urandom"设备不会阻塞，但是可能出现获取失败
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

    /// 获取失败后就只有利用非真随机数获取
    if (dwReadLen < dwLen)
    {
        Rand((BYTE *)pBuf + dwReadLen, dwLen - dwReadLen);
    }
}

/*******************************************************
  函 数 名: CDevRandom::Rand
  描    述: 模拟随机数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDevRandom::Rand(void *pBuf, DWORD dwLen)
{
    DWORD dwReadLen = 0;

    while (dwReadLen < dwLen)
    {
        *((BYTE *)pBuf + dwReadLen) = (BYTE)(rand() % 256);
        dwReadLen++;
    }
}

/*******************************************************
  函 数 名: objRandom::CreateInstance
  描    述: 创建随机数实例
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objRandom *objRandom::CreateInstance(const char *file, int line)
{
    #undef new
    return new (file, line) CDevRandom();
    #define new new(__FILE__, __LINE__)
}

/*******************************************************
  函 数 名: objRandom::~objRandom
  描    述: 基类析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objRandom::~objRandom()
{
}


/// -------------------------------------------------
/// 实现获取调用栈操作
/// -------------------------------------------------
#include <execinfo.h>

#define CALLSTACK_DEPTH_MAX 10

/*******************************************************
  函 数 名: ShowCallStack
  描    述: 显示调用栈
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
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

#undef free

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

#define free(ptr) DCOP_FreeEx(ptr, __FILE__, __LINE__)
}


/// -------------------------------------------------
/// 动态库加载操作
/// -------------------------------------------------
#include <dlfcn.h>

/// -------------------------------------------------
/// linux 动态库
/// -------------------------------------------------
class CDynamicLibrary : public objDynamicLoader
{
public:
    CDynamicLibrary();
    ~CDynamicLibrary();

    DWORD Load(const char *dllFile);
    DWORD Unload();

    void *FindSymbol(const char *symName);

    void SetErrLog(LOG_PRINT logPrint, LOG_PARA logPara);

private:
    void *m_handle;
    LOG_PRINT m_logPrint;
    LOG_PARA m_logPara;
};

/*******************************************************
  函 数 名: CDynamicLibrary::CDynamicLibrary
  描    述: CDynamicLibrary构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDynamicLibrary::CDynamicLibrary()
{
    m_handle = NULL;
    m_logPrint = PrintToConsole;
    m_logPara = 0;
}

/*******************************************************
  函 数 名: CDynamicLibrary::~CDynamicLibrary
  描    述: CDynamicLibrary析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDynamicLibrary::~CDynamicLibrary()
{
    (void)Unload();
}

/*******************************************************
  函 数 名: CDynamicLibrary::Load
  描    述: 加载
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDynamicLibrary::Load(const char *dllFile)
{
    if (!dllFile || !(*dllFile))
    {
        return FAILURE;
    }

    if (m_handle)
    {
        return FAILURE;
    }

    void *pf = dlopen(dllFile, RTLD_NOW | RTLD_GLOBAL);
    if (!pf)
    {
        PrintLog(dlerror(), m_logPrint, m_logPara);
        return FAILURE;
    }

    m_handle = pf;
    return SUCCESS;
}

/*******************************************************
  函 数 名: CDynamicLibrary::Unload
  描    述: 卸载
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDynamicLibrary::Unload()
{
    if (!m_handle)
    {
        return FAILURE;
    }

    int rc = dlclose(m_handle);
    if (rc)
    {
        PrintLog(dlerror(), m_logPrint, m_logPara);
        return FAILURE;
    }

    m_handle = NULL;
    return SUCCESS;
}

/*******************************************************
  函 数 名: CDynamicLibrary::~CDynamicLibrary
  描    述: CDynamicLibrary析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void *CDynamicLibrary::FindSymbol(const char *symName)
{
    if (!symName || !(*symName))
    {
        return NULL;
    }

    if (!m_handle)
    {
        return NULL;
    }

    void *pSymAddr = dlsym(m_handle, symName);
    if (!pSymAddr)
    {
        PrintLog(dlerror(), m_logPrint, m_logPara);
        return NULL;
    }

    return pSymAddr;
}

/*******************************************************
  函 数 名: CDynamicLibrary::~CDynamicLibrary
  描    述: CDynamicLibrary析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDynamicLibrary::SetErrLog(LOG_PRINT logPrint, LOG_PARA logPara)
{
    m_logPrint = (logPrint)? logPrint : PrintToConsole;
    m_logPara = (logPrint)? logPara : 0;
}

/*******************************************************
  函 数 名: objDynamicLoader::CreateInstance
  描    述: 创建动态加载实例
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objDynamicLoader *objDynamicLoader::CreateInstance(const char *file, int line)
{
    #undef new
    return new (file, line) CDynamicLibrary();
    #define new new(__FILE__, __LINE__)
}

/*******************************************************
  函 数 名: objDynamicLoader::~objDynamicLoader
  描    述: 基类析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objDynamicLoader::~objDynamicLoader()
{
}


#endif // #if DCOP_OS == DCOP_OS_LINUX

