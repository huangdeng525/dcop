/// -------------------------------------------------
/// ObjDispatch_if.h : 消息分发器对象公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJDISPATCH_IF_H_
#define _OBJDISPATCH_IF_H_

#include "Object_if.h"
#include "msg.h"


/// -------------------------------------------------
/// 默认消息长度(使用申请时的消息长度)
/// -------------------------------------------------
#define DISPATCH_LENGTH_DEFAULT             0xFFFFFFFF


/// -------------------------------------------------
/// 定义IDispatch版本号
/// -------------------------------------------------
INTF_VER(IDispatch, 1, 0, 0);


/// -------------------------------------------------
/// 消息分发接口
/// -------------------------------------------------
interface IDispatch : public IObject
{
    /// 获取MTU大小
    virtual DWORD GetMTU(
                        ) = 0;

    /// 异步非阻塞发送消息(不保证对端是否收到)
    virtual DWORD Send(objMsg *message) = 0;

    /// 发送消息后等待回应(阻塞直到收到对端响应)
    virtual DWORD SendAndWait(
                        objMsg *request, 
                        objMsg **response, 
                        DWORD waittime
                        ) = 0;
};


#endif // #ifndef _OBJDISPATCH_IF_H_

