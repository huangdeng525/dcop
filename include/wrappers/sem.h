/// -------------------------------------------------
/// sem.h : 信号量定义公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _SEM_H_
#define _SEM_H_

#include "dcop.h"


/// 创建锁
#define DCOP_CreateLock() \
    objLock::CreateInstance(__FILE__, __LINE__)


/// 锁对象(互斥信号量)
class objLock
{
public:
    static objLock *CreateInstance(const char *file,
                        int line);
    virtual ~objLock() = 0;

    virtual void Enter() = 0;

    virtual void Leave() = 0;
};

/// 自动锁
#define AutoLock(x) AutoLockLine(x, __LINE__)
#define AutoLockLine(x, line) AutoLockLineEx(x, line)
#define AutoLockLineEx(x, line) AutoLockEx __tmp_##line(x)
class AutoLockEx
{
public:
    AutoLockEx() {m_pLock = 0;}
    AutoLockEx(objLock *pLock) {m_pLock = pLock; if (m_pLock) m_pLock->Enter();}
    ~AutoLockEx() {if (m_pLock) m_pLock->Leave(); m_pLock = 0;}
private:
    objLock *m_pLock;
};


/// 创建事件
#define DCOP_CreateEvent(bHaveEvent) \
    objEvent::CreateInstance(bHaveEvent, __FILE__, __LINE__)


/// 事件对象(二进制信号量)
class objEvent
{
public:
    static objEvent *CreateInstance(bool bHaveEvent,
                        const char *file,
                        int line);
    virtual ~objEvent() = 0;

    virtual DWORD Send() = 0;

    virtual DWORD Recv(
                        DWORD waitMilliseconds = OSWAIT_FOREVER
                        ) = 0;

};


/// 创建计数器
#define DCOP_CreateCounter(initCount, maxCount) \
    objCounter::CreateInstance(initCount, maxCount, __FILE__, __LINE__)


/// 计数器对象(计数信号量)
class objCounter
{
public:
    static objCounter *CreateInstance(DWORD initCount,
                        DWORD maxCount,
                        const char *file,
                        int line);
    virtual ~objCounter() = 0;

    virtual DWORD Take(
                        DWORD waitMilliseconds = OSWAIT_FOREVER
                        ) = 0;

    virtual DWORD Give(
                        DWORD giveCount = 1
                        ) = 0;

};


#endif // #ifndef _SEM_H_

