/// -------------------------------------------------
/// session_if.h : 会话管理公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _SESSION_IF_H_
#define _SESSION_IF_H_

#include "Object_if.h"
#include "ObjTimer_if.h"


/////////////////////////////////////////////////////
///                 '会话管理'说明
/// -------------------------------------------------
/// '会话'是在线用户登录系统后，在系统中访问各种资源的
/// 一个临时凭证。
/// 1) '会话凭证'在建立时由一个DWORD类型的'会话ID'组成
/// 2) '会话ID'在服务器当前会话中不会重复
/// 3) '会话ID'在访问内部资源时会被传来传去，可能再次
///    到达'会话管理'中就已经过期，但是没有关系，查找
///    会话虽然是只通过ID索引，但是验证会话是否正确是
///    需要通过消息中携带的'会话ID'+'用户ID'+'终端ID'
///    一起作为验证标准，如果不匹配，该会话就会被丢弃
/// 4) '会话ID'也会被空闲超时后退出，如果会话有交互的
///    话可以通过'更新会话'接口来防止超时，不过调用该
///    接口时，请不要随时进行更新，以免给会话管理带来
///    承重负担
/// 5) '用户ID'由'用户管理'维护，是永久存在的，除非被
///    手工从数据库中删除
/// 6) '终端ID'标识是用户登录的终端，是负责对应终端的
///    接入模块的对象ID
/// 7) '会话管理'不提供单次请求和响应管理，因为如果提
///    供该功能的话，会不断进出'会话管理'，大家很容易
///    造成拥塞；响应应该由各个接入模块自己创建请求缓
///    冲池来进行(已经封装好，其实调用很简单)
/////////////////////////////////////////////////////


/// -------------------------------------------------
/// 定义ISession版本号
/// -------------------------------------------------
INTF_VER(ISession, 1, 0, 0);


/// -------------------------------------------------
/// 用户接口定义
/// -------------------------------------------------
interface ISession : public IObject
{
    /// 信息长度
    static const WORD INFO_SIZE = 22;

    /// 节点
    struct NODE
    {
        DWORD SessID;
        DWORD UserID;
        DWORD UserGroup;
        DWORD TTY;
        ITimer::Node Timer;
        DWORD IP;
        WORD Port;
    };

    /// 创建会话
    virtual DWORD CreateSession(
                        DWORD dwUserID,             // 用户ID
                        DWORD dwUserGroup,          // 用户组
                        DWORD dwTTY,                // 终端ID
                        DWORD dwRemoteIP,           // 远程IP
                        WORD wRemotePort,           // 远程端口
                        DWORD &rdwSessionID         // 会话ID
                        ) = 0;

    /// 删除会话
    virtual DWORD DeleteSession(
                        DWORD dwSessionID           // 会话ID
                        ) = 0;

    /// 更新会话
    /// [使之不要超时]
    virtual DWORD UpdateSession(
                        DWORD dwSessionID           // 会话ID
                        ) = 0;

    /// 更新用户信息
    virtual DWORD UpdateUserID(
                        DWORD dwSessionID,          // 会话ID
                        DWORD dwUserID,             // 用户ID
                        DWORD dwUserGroup           // 用户组
                        ) = 0;

    /// 查找会话
    /// [通过远程IP和远程端口]
    virtual DWORD FindSession(
                        DWORD dwRemoteIP,           // 远程IP
                        WORD wRemotePort,           // 远程端口
                        NODE &rNode                 // 会话节点
                        ) = 0;

    /// 获取会话
    virtual DWORD GetSession(
                        DWORD dwSessionID,          // 会话ID
                        NODE &rNode                 // 会话节点
                        ) = 0;

    /// 设置会话信息
    virtual DWORD SetSessionInfo(
                        DWORD dwSessionID,          // 会话ID
                        char szInfo[INFO_SIZE]      // 会话信息
                        ) = 0;

    /// 获取会话信息
    virtual DWORD GetSessionInfo(
                        DWORD dwSessionID,          // 会话ID
                        char szInfo[INFO_SIZE]      // 会话信息
                        ) = 0;
};


#endif // #ifndef _SESSION_IF_H_


