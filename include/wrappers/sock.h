/// -------------------------------------------------
/// sockType.h : socket封装类公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _SOCK_H_
#define _SOCK_H_

#include "dcop.h"


/// IP地址大小(IPv4)
#define OSSOCK_IPSIZE                       16

/// 默认Socket端口
#define OSSOCK_DEFAULT_PORT                 34418

/// 默认Socket最大传输单元
#define OSSOCK_DEFAULT_MTU                  (16*1024)

/// 默认Socket每次阻塞时间
#define OSSOCK_DEFAULT_BLOCKTIME            1000

/// 默认Socket任务栈大小
#define OSSOCK_DEFAULT_TASK_STACKSIZE       (32*1024)

/// 默认Socket发送Buf大小
#define OSSOCK_DEFAULT_SENDBUFLEN           40960

/// 默认Socket接收Buf大小
#define OSSOCK_DEFAULT_RECVBUFLEN           40960

/// 默认Socket发送重试次数
#define OSSOCK_DEFAULT_SENDRETRYTIMES       20

/// 默认Socket监听队列个数(TCPServer)
#define OSSOCK_DEFAULT_LISTENNUM            10

/// 单个网络应用最大任务数量
#define OSSOCK_LANAPP_TASK_MAX_COUNT        255


/// 创建Sock对象
#define DCOP_CreateSock(dwSockFlag, cpszIP, wPort) \
    objSock::CreateInstance(dwSockFlag, cpszIP, wPort, __FILE__, __LINE__)


/// -------------------------------------------------
/// sock对象
/// -------------------------------------------------
class objSock
{
public:

    /// Socket句柄类型
    typedef unsigned int SOCKET;                        // SOCK句柄

    /// SOCK类型
    typedef enum tagOSSOCK_FLAG                         // Socket类型
    {
        OSSOCK_NONE = 0,                                // 没有创建套接字
        OSSOCK_UDP,                                     // UDP方式的套接字
        OSSOCK_TCPSERVER,                               // TCP服务端创建的套接字
        OSSOCK_TCPACCEPT,                               // TCP服务端接收客户端连接的套接字
        OSSOCK_TCPCLIENT                                // TCP客户端创建的套接字
    }OSSOCK_FLAG;

public:
    static objSock *CreateInstance(DWORD dwSockFlag, 
                        const char *cpszIP, WORD wPort, 
                        const char *file,
                        int line);
    virtual ~objSock() = 0;

    /// ---------------------------------------------
    /// 获取套接字参数
    /// ---------------------------------------------
    virtual DWORD       dwGetSockFlag() = 0;
    virtual DWORD       dwGetHostID() = 0;
    virtual const char *cszGetHostIP() = 0;
    virtual WORD        wGetHostPort() = 0;
    virtual int         iGetSendBufLen() = 0;
    virtual int         iGetRecvBufLen() = 0;
    virtual int         iGetLingerTime() = 0;
    virtual int         iGetListenNum() = 0;
    virtual bool        bGetTcpNoDelay() = 0;
    virtual bool        bGetUdpBind() = 0;
    virtual bool        bGetUdpBoardcast() = 0;
    virtual int         iGetSendTryTimes() = 0;
    virtual SOCKET      sGetSock() = 0;
    virtual const void *cpGetSockAddr() = 0;
    virtual DWORD       dwGetSockAddrSize() = 0;

    /// ---------------------------------------------
    /// 设置套接字参数
    /// ---------------------------------------------
    virtual void        vSetSockFlag(DWORD dwFlag) = 0;
    virtual void        vSetHostID(DWORD dwHostID) = 0;
    virtual void        vSetHostIP(const char *cpszIP) = 0;
    virtual void        vSetHostPort(WORD wPort) = 0;
    virtual void        vSetSendBufLen(int iSendBufLen) = 0;
    virtual void        vSetRecvBufLen(int iRecvBufLen) = 0;
    virtual void        vSetLingerTime(int iLingerTime) = 0;
    virtual void        vSetListenNum(int iListenNum) = 0;
    virtual void        vSetTcpNoDelay(bool bTcpNoDelay) = 0;
    virtual void        vSetUdpBind(bool bUdpBind) = 0;
    virtual void        vSetUdpBoardcast(bool bUdpBoardcast) = 0;
    virtual void        vSetSendTryTimes(int iSendTryTimes) = 0;
    virtual void        vSetSock(SOCKET s) = 0;
    virtual void        vSetSockAddr(const void *cpsockaddr) = 0;


    /// ---------------------------------------------
    ///    打开 关闭 - begin
    /// ---------------------------------------------

    virtual DWORD Open() = 0;
    virtual DWORD Close() = 0;
    virtual DWORD Shut() = 0;

    /// ---------------------------------------------
    ///    打开 关闭 - end
    /// ---------------------------------------------


    /// ---------------------------------------------
    ///    TCP Server 使用 - begin
    /// ---------------------------------------------

    virtual DWORD Accept(
                        objSock *& rpCltSock,           // 输出远端Sock对象
                        bool bTimeControl = false,      // 是否进行时间控制
                        DWORD dwWaitTime = 0            // 等待时间值
                        /////////////////////////////////////////
                        /// bTimeControl == false 不进行时间控制
                        ///     即利用套接字本身的阻塞时间
                        /////////////////////////////////////////
                        ) = 0;

    /// ---------------------------------------------
    ///    TCP Server 使用 - end
    /// ---------------------------------------------


    /// ---------------------------------------------
    ///     TCP Client 使用 - begin
    /// ---------------------------------------------

    /// 连接远端服务器
    virtual DWORD Connect(
                        const char *pDstIP,             // 连接目的地址
                        WORD wDstPort,                  // 连接目的端口
                        bool bAsynCall = false,         // 异步调用connect
                        bool bTimeControl = false,      // 是否进行时间控制
                        DWORD dwWaitTime = 0            // 等待时间值
                        ) = 0;

    /// 异步调用connect后继续完成连接
    virtual DWORD FinishConnect() = 0;

    /// ---------------------------------------------
    ///     TCP Client 使用 - end
    /// ---------------------------------------------


    /// ---------------------------------------------
    ///     TCP 公共使用 - begin
    /// ---------------------------------------------

    /// TCP发送函数
    virtual DWORD Send(
                        const void *cpBuf,              // 输入数据缓冲区地址
                        DWORD dwBufLen,                 // 输入数据缓冲区的大小
                        DWORD *pdwSent = 0,             // 已发送出去的数据
                        bool bTimeControl = false,      // 是否进行时间控制
                        DWORD dwWaitTime = 0            // 等待时间值
                        ) = 0;

    /// TCP接收数据函数
    virtual DWORD Recv(
                        void *pBuf,                     // 输出数据
                        DWORD dwBufLen,                 // 输入本缓冲区的大小
                        DWORD& rdwRecvLen,              // 输出收到消息的大小
                        bool bTimeControl = false,      // 是否进行时间控制
                        DWORD dwWaitTime = 0            // 等待时间值
                        ) = 0;

    /// ---------------------------------------------
    ///     TCP 公共使用 - end
    /// ---------------------------------------------


    /// ---------------------------------------------
    ///     UDP 使用 - begin
    /// ---------------------------------------------

    /// UDP发送函数
    virtual DWORD Sendto(
                        const void *cpBuf,              // 输入数据缓冲区地址
                        DWORD dwBufLen,                 // 输入数据缓冲区的大小
                        const char *pDstIP,             // 发送目的地址
                        WORD wDstPort                   // 发送目的端口
                        ) = 0;

    /// UDP接收函数
    virtual DWORD Recvfrom(
                        void *pBuf,                     // 输出数据
                        DWORD dwBufLen,                 // 输入本缓冲区的大小
                        DWORD& rdwRecvLen,              // 输出收到消息的大小
                        char szSrcIP[OSSOCK_IPSIZE],    // 发送源地址
                        WORD &rwSrcPort                 // 发送源端口
                        ) = 0;

    /// ---------------------------------------------
    ///     UDP 使用 - end
    /// ---------------------------------------------

    /// ---------------------------------------------
    /// 静态函数 - begin
    /// ---------------------------------------------

    /// 根据主机名获取IP地址字符串
    static void GetIPStringByName(const char *cszName, char szIP[OSSOCK_IPSIZE]);

    /// 根据主机名获取IP地址值
    static DWORD GetIPValueByName(const char *cszName);

    /// 根据IP地址字符串获取IP地址值
    static DWORD GetIPValueByString(const char *cszIP);

    /// 根据IP地址值获取IP地址字符串
    static void GetIPStringByValue(DWORD dwIP, char szIP[OSSOCK_IPSIZE]);
    
    /// ---------------------------------------------
    /// 静态函数 - end
    /// ---------------------------------------------

                    
};


/// -------------------------------------------------
/// 网络应用对象
/// -------------------------------------------------
class objLanApp
{
public:
    static objLanApp *CreateInstance();                         // 获取实例
    virtual ~objLanApp() = 0;                                   // 释放实例

    /// 服务器收到一个客户端连接
    typedef DWORD (*FUNC_ON_ACCEPT)(
                        DWORD dwChannelID,
                        objSock *pServerSock,
                        objSock *pAcceptSock,
                        void *pUserArg);

    /// 客户端成功连接上服务器
    typedef DWORD (*FUNC_ON_CONNECT)(
                        DWORD dwChannelID,
                        objSock *pClientSock,
                        const char *cszRemoteIP,
                        WORD wRemotePort,
                        void *pUserArg);

    /// 和远端连接断开
    typedef void (*FUNC_ON_DISCONNECT)(
                        DWORD dwChannelID,
                        objSock *pSock,
                        const char *cszRemoteIP,
                        WORD wRemotePort,
                        void *pUserArg);

    /// 收到数据
    typedef void (*FUNC_ON_RECV)(
                        DWORD dwChannelID,
                        objSock *pSock,
                        void *pFrameBuf,
                        DWORD dwFrameLen,
                        const char *cszRemoteIP,
                        WORD wRemotePort,
                        void *pUserArg);

    /// 网络事件处理
    struct LanEventProc
    {
        FUNC_ON_ACCEPT fnAcceptProc;
        void *pAcceptProcPara;
        FUNC_ON_CONNECT fnConnectProc;
        void *pConnectProcPara;
        FUNC_ON_DISCONNECT fnDisconnectProc;
        void *pDisconnectProcPara;
        FUNC_ON_RECV fnRecvProc;
        void *pRecvProcPara;
    };

    /// 判断是否是数据帧
    typedef int (*FUNC_B_FRAME)(
                        void *pBuf,
                        DWORD dwLen,
                        void *pUserArg);

    /// 转换字节序
    typedef void (*FUNC_CHANGE_BYTES_ORDER)(
                        void *pBuf,
                        DWORD dwLen,
                        void *pUserArg);

    /// 网络数据帧处理
    struct LanFrameProc
    {
        FUNC_B_FRAME fnBFrameProc;
        void *pBFrameProcPara;
        FUNC_CHANGE_BYTES_ORDER fnChangeBytesOrderProc;
        void *pChangeBytesOrderProcPara;
    };

    /// 日志打印回调
    typedef void (*FUNC_LOG_PRINT)(
                        const char *cszLogInfo,
                        const char *pFile,
                        DWORD dwLine,
                        void *pUserArg);

    /// 网络日志处理
    struct LanLogProc
    {
        FUNC_LOG_PRINT fnLogProc;
        void *pLogProcPara;
    };

public:

    /// 获取本端ID
    virtual DWORD GetLocalID(
                        ) = 0;

    /// 获取通道套接字
    virtual objSock *GetChannel(
                        DWORD dwChannelID
                        ) = 0;

    /// 添加TCPServer通道
    virtual DWORD AddTCPServer(
                        DWORD dwChannelID,
                        WORD wLocalPort
                        ) = 0;

    /// 添加TCPClient通道
    virtual DWORD AddTCPClient(
                        DWORD dwChannelID,
                        const char *szRemoteIP,
                        WORD wRemotePort
                        ) = 0;

    /// 添加UDP通道
    virtual DWORD AddUDP(
                        DWORD dwChannelID,
                        bool bBind,
                        WORD wLocalPort,
                        bool bBoardcast
                        ) = 0;

    /// 删除通道
    virtual void DelChannel(
                        DWORD dwChannelID
                        ) = 0;

    /// 启动应用
    virtual DWORD Start(
                        const char *cszAppName,
                        DWORD dwLocalID,
                        DWORD dwTaskCount,
                        LanEventProc *pEventProc,
                        LanFrameProc *pFrameProc = 0,
                        LanLogProc *pLogProc = 0
                        ) = 0;

    /// 停止应用
    virtual void Stop() = 0;

};


#endif // #ifndef _SOCK_H_

