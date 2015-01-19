/// -------------------------------------------------
/// ObjResponse_main.h : 响应器对象私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJRESPONSE_MAIN_H_
#define _OBJRESPONSE_MAIN_H_

#define INC_SET

#include "ObjResponse_if.h"
#include "Manager_if.h"
#include "ObjResponse_pool.h"
#include "ObjTimer_if.h"


/////////////////////////////////////////////////////
/// [说明] 这里定时轮的使用方法和定时器里面是一样的，
/// 使用4个轮子来分别表示秒、分、时、天；
/// 也可以使用一个轮子，同时在定时器值里面增加一个轮子
/// 周期的倍数计数: 比如轮子周期有32秒，现在有超时为
/// 100秒的节点，那么则在定时器中增加一个倍数100/32,
/// 再取余数100%32，得出定时器轮的槽位。
/// 为什么取32秒，因为:
///     1. 原值 '>>5' 刚好就获得了倍数
///     2. 原值加上当前槽位刻度，再 '& 0x1F' 就是余数，
///        也就是需要加入的定时器槽位
/////////////////////////////////////////////////////


/// 命令响应管理
class CResponse : public IResponse
{
public:
    /// 请求缓冲池SET
    typedef std::set<CResponsePool *> SET_RESPONSE;
    typedef SET_RESPONSE::iterator IT_RESPONSE;

public:
    CResponse(Instance *piParent, int argc, char **argv);
    ~CResponse();

    DCOP_DECLARE_INSTANCE;
    DCOP_DECLARE_IOBJECT;

    DCOP_DECLARE_IOBJECT_MSG_HANDLE;

    DWORD Init(IObject *root, int argc, void **argv);
    void Fini();

    void OnStart(objMsg *msg);
    void OnFinish(objMsg *msg);
    void OnTimer1s(objMsg *msg);

    IPool *CreatePool(IObject *owner, DWORD count);

    void DestroyPool(IPool *pool);

private:
    SET_RESPONSE m_requests;
    IManager *m_piManager;
    ITimer *m_piTimer;
    ITimer::Handle m_hTimer1s;
};


#endif // #ifndef _OBJRESPONSE_MAIN_H_

