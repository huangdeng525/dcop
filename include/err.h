/// -------------------------------------------------
/// err.h : 错误码定义公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _DCOP_ERR_H_
#define _DCOP_ERR_H_

/// 错误码基址(需要左移16个bit)
enum DCOP_ERRCODE
{
    SUCCESS = 0,

    ERRCODE_OS,                                 // 0x0001 - 基础相关错误码
    ERRCODE_TASK,                               // 0x0002 - 任务相关错误码
    ERRCODE_SEM,                                // 0x0003 - 信号量相关错误码
    ERRCODE_MSG,                                // 0x0004 - 消息相关错误码
    ERRCODE_SOCK,                               // 0x0005 - 套接字相关错误码
    ERRCODE_IO,                                 // 0x0006 - 输入输出相关错误码

    ERRCODE_END,

    ERRCODE_USER = 0x000F                       // 0x000F - 用户自定义错误码
};

#define FAILURE (__LINE__)
#define TIMEOUT 1

/// 操作系统错误码
enum DCOP_ERRCODE_OS
{
    ERRCODE_OS_BASE         = (ERRCODE_OS) << 16,

    ERRCODE_OS_END
};

/// 任务错误码
enum DCOP_ERRCODE_TASK
{
    ERRCODE_TASK_BASE       = (ERRCODE_TASK) << 16,

    ERRCODE_TASK_WRONG_HANDLE,                  // 0x00020001 - 错误句柄
    ERRCODE_TASK_CREATE_FAIL,                   // 0x00020002 - 创建任务失败
    ERRCODE_TASK_DESTROY_FAIL,                  // 0x00020003 - 销毁任务失败
    

    ERRCODE_TASK_END
};

/// 信号量错误码
enum DCOP_ERRCODE_SEM
{
    ERRCODE_SEM_BASE        = (ERRCODE_SEM) << 16,
    ERRCODE_SEM_WRONG_HANDLE,                   // 0x00030001 - 错误句柄
    ERRCODE_SEM_WAIT_TIMEOUT,                   // 0x00030002 - 等待超时
    ERRCODE_SEM_OVER_MAXCOUNT,                  // 0x00030003 - 超过最大值

    ERRCODE_SEM_END
};

/// 消息队列错误码
enum DCOP_ERRCODE_MSG
{
    ERRCODE_MSG_BASE        = (ERRCODE_MSG) << 16,

    ERRCODE_MSG_QUE_FULL,                       // 消息队列满
    ERRCODE_MSG_QUE_EMPTY,                      // 消息队列空
    ERRCODE_MSG_SENDPRIO_ERROR,                 // 发送优先级错误
    ERRCODE_MSG_NOT_FRAME_HEADER,               // 不是消息帧头部(BUF格式不对)
    ERRCODE_MSG_PACKET_NOT_HEADER,              // 不是命令包头部(CMD格式不对)
    ERRCODE_MSG_PACKET_FULL,                    // 命令包已满
    ERRCODE_MSG_PACKET_NO_SPACE,                // 命令包无空闲空间

    ERRCODE_MSG_END
};

/// 套接字错误码
enum DCOP_ERRCODE_SOCK
{
    ERRCODE_SOCK_BASE       = (ERRCODE_SOCK) << 16,

    ERRCODE_SOCK_FLAG_NON_NONE,                 // 标识不为空
    ERRCODE_SOCK_FLAG_ERR_VALUE,                // 标识错误
    ERRCODE_SOCK_PARA_ERR_VALUE,                // 参数错误
    ERRCODE_SOCK_INIT_FAILURE,                  // 初始化失败
    ERRCODE_SOCK_CREATE_FAILURE,                // 创建失败
    ERRCODE_SOCK_INVALID_SOCKET,                // 不可利用的套接字
    ERRCODE_SOCK_SELECT_ERROR,                  // 套接字select调用错误
    ERRCODE_SOCK_SELECT_TIMEOUT,                // 套接字select调用错误
    ERRCODE_SOCK_SEND_ERROR,                    // 套接字send(sendto)调用错误
    ERRCODE_SOCK_RECV_ERROR,                    // 套接字recv(recvfrom)调用错误
    ERRCODE_SOCK_INPUT_ERROR,                   // 输入参数错误
    ERRCODE_SOCK_CONN_CLSOED,                   // 连接已经被关闭
    ERRCODE_SOCK_ACCEPT_ERROR,                  // 接收连接错误
    ERRCODE_SOCK_CONN_ERROR,                    // 连接错误
    ERRCODE_SOCK_CLTSOCK_ERROR,                 // 客户套接字错误

    ERRCODE_SOCK_END
};

/// IO错误码
enum DCOP_ERRCODE_IO
{
    ERRCODE_IO_BASE         = (ERRCODE_IO) << 16,

    ERRCODE_IO_NO_RIGHT_TO_OPERATE,             // 操作权限不够

    ERRCODE_IO_END
};


#endif // #ifndef _DCOP_ERR_H_

