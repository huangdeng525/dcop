/// -------------------------------------------------
/// task.h : 操作系统task接口公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _TASK_H_
#define _TASK_H_

#include "dcop.h"


/// -------------------------------------------------
/// 创建任务的宏
/// -------------------------------------------------
#define DCOP_CreateTask(cszName, pEntry, dwStackSize, dwPriority, pPara) \
    objTask::CreateInstance(cszName, pEntry, dwStackSize, dwPriority, pPara, __FILE__, __LINE__)

/// -------------------------------------------------
/// 任务对象(任务从入口退出时会析构本任务对象)
/// -------------------------------------------------
class objTask
{
public:
    class IPara                                                 // 参数对象
    {
    public:
        virtual ~IPara();                                       // 任务从入口退出时会析构该参数对象
    };

    typedef void (*OSTASK_ENTRY)(IPara *para);                  // 任务入口函数原型

    enum OSTASK_PRIORITY                                        // 任务优先级
    {
        OSTASK_PRIORITY_LOWEST,                                 // 最低
        OSTASK_PRIORITY_LOWER,                                  // 较低
        OSTASK_PRIORITY_NORMAL,                                 // 普通
        OSTASK_PRIORITY_HIGHER,                                 // 较高
        OSTASK_PRIORITY_HIGHEST                                 // 最高
    };

public:
    static objTask *CreateInstance(const char *cszName,
                        OSTASK_ENTRY pEntry,
                        DWORD dwStackSize,
                        DWORD dwPriority,
                        IPara *pPara,
                        const char *file,
                        int line);
    virtual ~objTask() = 0;

    virtual const char *GetName() = 0;
    virtual DWORD GetID() = 0;

    static void Delay(DWORD delayMilliseconds);
    static objTask *Current();
};


/// -------------------------------------------------
/// 原子操作对象
/// -------------------------------------------------
class objAtomic
{
public:
    typedef intptr_t T;

public:
    static T Inc(T &val);
    static T Dec(T &val);
    static T Add(T &val, T add);
    static T Sub(T &val, T sub);
    static T Set(T &val, T set);
    static bool CAS(T &val, T cmp, T swap);
};


/// -------------------------------------------------
/// 自旋锁(利用原子操作完成锁状态/不能嵌套使用)
/// -------------------------------------------------
class objSpinLock
{
public:
    objSpinLock() : m_lock(0) {}
    ~objSpinLock() {}

    void Lock() { while (!objAtomic::CAS(m_lock, 0, 1)) {} }
    void Unlock() { (void)objAtomic::Set(m_lock, 0); }

private:
    objAtomic::T m_lock;
};


/// -------------------------------------------------
/// 自动自旋锁
/// -------------------------------------------------
#define AutoSpinLock(x) AutoSpinLockLine(x, __LINE__)
#define AutoSpinLockLine(x, line) AutoSpinLockLineEx(x, __LINE__)
#define AutoSpinLockLineEx(x, line) AutoSpinLockEx __tmp_##line(x)
class AutoSpinLockEx
{
public:
    AutoSpinLockEx() {m_pLock = 0;}
    AutoSpinLockEx(objSpinLock *pLock) {m_pLock = pLock; if (m_pLock) m_pLock->Lock();}
    ~AutoSpinLockEx() {if (m_pLock) m_pLock->Unlock(); m_pLock = 0;}
private:
    objSpinLock *m_pLock;
};


/// -------------------------------------------------
/// 创建随机数对象
/// -------------------------------------------------
#define DCOP_CreateRandom() \
    objRandom::CreateInstance(__FILE__, __LINE__)

/// -------------------------------------------------
/// 随机数对象
/// -------------------------------------------------
class objRandom
{
public:
    static objRandom *CreateInstance(const char *file,
                        int line);
    virtual ~objRandom() = 0;

    /// 生成随机数
    virtual void Gen(void *pBuf, DWORD dwLen) = 0;
};


/// -------------------------------------------------
/// 创建动态加载对象
/// -------------------------------------------------
#define DCOP_CreateDynamicLoader() \
    objDynamicLoader::CreateInstance(__FILE__, __LINE__)

/// -------------------------------------------------
/// 动态加载
/// -------------------------------------------------
class objDynamicLoader
{
public:
    static objDynamicLoader *CreateInstance(const char *file,
                        int line);
    virtual ~objDynamicLoader() = 0;

    /// 加载和卸载
    virtual DWORD Load(const char *dllFile) = 0;
    virtual DWORD Unload() = 0;

    /// 查找符号
    virtual void *FindSymbol(const char *symName) = 0;

    /// 设置错误信息回调
    virtual void SetErrLog(LOG_PRINT logPrint, LOG_PARA logPara) = 0;
};


#endif // #ifndef _TASK_H_

