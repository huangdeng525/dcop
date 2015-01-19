/// -------------------------------------------------
/// ObjResponse_if.h : 响应器对象公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _OBJRESPONSE_IF_H_
#define _OBJRESPONSE_IF_H_

#include "Object_if.h"
#include "BaseMessage.h"


/// -------------------------------------------------
/// 定义IResponse版本号
/// -------------------------------------------------
INTF_VER(IResponse, 1, 0, 0);


/// -------------------------------------------------
/// 命令响应接口
/// -------------------------------------------------
interface IResponse : public IObject
{
    /// 请求缓冲池(供请求者使用，在本地缓冲哪些请求未被响应)
    interface IPool
    {
        /// 发送请求时
        virtual DWORD OnReq(
                        DCOP_SESSION_HEAD *pSession,
                        DWORD dwMsgType,
                        DWORD dwSrcID,
                        DWORD dwDstID,
                        DWORD dwRspMsgType,
                        DWORD dwTimeout,
                        DWORD dwSendTryTimes
                        ) = 0;

        /// 接收响应时
        virtual DWORD OnRsp(
                        DCOP_SESSION_HEAD *pSession
                        ) = 0;
    };


    /// 创建缓冲区
    virtual IPool *CreatePool(
                        IObject *owner,
                        DWORD count
                        ) = 0;

    /// 删除缓冲区
    virtual void DestroyPool(
                        IPool *pool
                        ) = 0;
};


#endif // #ifndef _OBJRESPONSE_IF_H_

