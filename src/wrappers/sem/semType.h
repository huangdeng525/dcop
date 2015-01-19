/// -------------------------------------------------
/// semType.h : 信号量定义私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _SEMTYPE_H_
#define _SEMTYPE_H_

#include "../osBase.h"
#include "sem.h"


/// 锁
class CLockBase : public objLock, private osBase
{
public:
    CLockBase(const char *file, int line);
    ~CLockBase();

    void Enter();

    void Leave();

private:
    void Initialize(const char *file, int line);
    void Delete();
    
};

/// 事件
class CEventBase : public objEvent, private osBase
{
public:
    CEventBase(bool bHaveEvent, const char *file, int line);
    ~CEventBase();
    
    DWORD Send();

    DWORD Recv(DWORD waitMilliseconds);

private:
    DWORD Create(bool bHaveEvent, const char *file, int line);
    DWORD Destroy();
};

/// 计数器
class CCounterBase : public objCounter, private osBase
{
public:
    CCounterBase(DWORD initCount, DWORD maxCount, const char *file, int line);
    ~CCounterBase();

    DWORD Take(DWORD waitMilliseconds);

    DWORD Give(DWORD giveCount);

private:
    DWORD Create(DWORD initCount, DWORD maxCount, const char *file, int line);
    DWORD Destroy();
};


#endif // #ifndef _SEMTYPE_H_

