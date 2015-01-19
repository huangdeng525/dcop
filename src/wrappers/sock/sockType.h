/// -------------------------------------------------
/// sockType.h : socket封装类公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _SOCKTYPE_IF_H_
#define _SOCKTYPE_IF_H_

#include "../osBase.h"
#include "sock.h"

#ifdef _MSC_VER
    #pragma warning(disable:4005)
    #pragma warning(disable:4244)
    #pragma warning(disable:4311)
    #pragma warning(disable:4312)
#endif

#if DCOP_OS == DCOP_OS_WINDOWS
#define FD_SETSIZE 1024 // windows下select默认是64，这里设置为最大的1024
#include <winsock2.h>

    /// ---------------------------------------------
    /// windows下设置非阻塞 : ioctlsocket(server_socket, FIONBIO, &ul); int ul = 1
    /// windows下send函数最后一个参数 : 一般设置为0
    /// windows下毫秒级时间获取 : GetTickCount()
    /// windows下编译连接库ws2_32.lib
    /// ---------------------------------------------

    #ifndef MSG_NOSIGNAL
        #define MSG_NOSIGNAL 0
    #endif

    #define close closesocket
    #define errno WSAGetLastError()
    #define ioctl(s, cmd, pt) ioctlsocket(s, cmd, (unsigned long *)(pt))

    inline int setnonblock(int sockFd)
    {
        int optVal = 1;
        return ioctl(sockFd, (int)FIONBIO, (int)(char *)&optVal);
    }

    #define socklen_t int

#elif DCOP_OS == DCOP_OS_LINUX
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

    /// ---------------------------------------------
    /// linux下设置非阻塞 : fcntl(server_socket, F_SETFL, O_NONBLOCK); <fcntl.h>
    /// linux下send函数最后一个参数 : 必须是后几个参数用到的socket中，值最大的数（整型）加1
    ///      另一种说法是设置为MSG_NOSIGNAL : 禁止send()函数向系统发送异常消息，系统不会出现BrokePipe
    /// linux下毫秒级时间获取 : gettimeofday()
    /// linux下连接是使用参数 : -lstdc (运行时需要libstdc++.so.5，可在/usr/lib目录中创建一个链接)
    /// ---------------------------------------------

    #ifndef INVALID_SOCKET
        #define INVALID_SOCKET (SOCKET)(~0)
    #endif

    #ifndef SOCKET_ERROR
        #define SOCKET_ERROR   (-1)
    #endif

    inline int setnonblock(int sockFd)
    {
        int optVal = fcntl(sockFd, F_GETFL, 0);
        return fcntl(sockFd, F_SETFL, optVal|O_NONBLOCK);
    }

#endif


class CSockBase : public objSock, private osBase
{
public:
    CSockBase();
    ~CSockBase();

    /// ---------------------------------------------
    /// 获取套接字参数
    /// ---------------------------------------------
    DWORD           dwGetSockFlag() {return m_dwSockFlag;}
    DWORD           dwGetHostID() {return m_dwHostID;}
    const char *    cszGetHostIP() {return m_szHostIP;}
    WORD            wGetHostPort() {return m_wHostPort;}
    int             iGetSendBufLen() {return m_iSendBufLen;}
    int             iGetRecvBufLen() {return m_iRecvBufLen;}
    int             iGetLingerTime() {return m_iLingerTime;}
    int             iGetListenNum() {return m_iListenNum;}
    bool            bGetTcpNoDelay() {return m_bTcpNoDelay;}
    bool            bGetUdpBind() {return m_bUdpBind;}
    bool            bGetUdpBoardcast() {return m_bUdpBoardcast;}
    int             iGetSendTryTimes() {return m_iSendTryTimes;}
    SOCKET          sGetSock() {return (SOCKET)(size_t)(hGetHandle());}
    const void *    cpGetSockAddr() {return &m_HostSockAddr;}
    DWORD           dwGetSockAddrSize() {return sizeof(m_HostSockAddr);}

    /// ---------------------------------------------
    /// 设置套接字参数
    /// ---------------------------------------------
    void            vSetSockFlag(DWORD dwFlag) {m_dwSockFlag = dwFlag;}
    void            vSetHostID(DWORD dwHostID) {m_dwHostID = dwHostID;}
    void            vSetHostIP(const char *cpszIP)
                    {
                        (void)memset(m_szHostIP, 0, sizeof(m_szHostIP));
                        if (cpszIP)
                        {
                            (void)strncpy(m_szHostIP, cpszIP, sizeof(m_szHostIP));
                            m_szHostIP[sizeof(m_szHostIP) - 1] = '\0';
                        }
                    }
    void            vSetHostPort(WORD wPort) {m_wHostPort = wPort;}
    void            vSetSendBufLen(int iSendBufLen) {m_iSendBufLen = iSendBufLen;}
    void            vSetRecvBufLen(int iRecvBufLen) {m_iRecvBufLen = iRecvBufLen;}
    void            vSetLingerTime(int iLingerTime) {m_iLingerTime = iLingerTime;}
    void            vSetListenNum(int iListenNum) {m_iListenNum = iListenNum;}
    void            vSetTcpNoDelay(bool bTcpNoDelay) {m_bTcpNoDelay = bTcpNoDelay;}
    void            vSetUdpBind(bool bUdpBind) {m_bUdpBind = bUdpBind;}
    void            vSetUdpBoardcast(bool bUdpBoardcast) {m_bUdpBoardcast = bUdpBoardcast;}
    void            vSetSendTryTimes(int iSendTryTimes) {m_iSendTryTimes = iSendTryTimes;}
    void            vSetSock(SOCKET s) {vSetHandle((OSHANDLE)s);}
    void            vSetSockAddr(const void *cpsockaddr)
                    {
                        m_HostSockAddr.sin_family = ((sockaddr_in *)cpsockaddr)->sin_family;
                        m_HostSockAddr.sin_port = ((sockaddr_in *)cpsockaddr)->sin_port;
                        m_HostSockAddr.sin_addr.s_addr = ((sockaddr_in *)cpsockaddr)->sin_addr.s_addr;
                        for (int i = 0; i < 8; i++)
                        {
                            m_HostSockAddr.sin_zero[i] = 0;
                        }
                    }

    DWORD Open();

    /// ---------------------------------------------
    ///    TCP Server 使用 - begin
    /// ---------------------------------------------

    DWORD Accept(
            objSock *& rpCltSock,           // 输出远端Sock对象
            bool bTimeControl = false,      // 是否进行时间控制
            DWORD dwWaitTime = 0            // 等待时间值
            /////////////////////////////////////////
            /// bTimeControl == false 不进行时间控制
            ///     即利用套接字本身的阻塞时间
            /////////////////////////////////////////
            );

    /// ---------------------------------------------
    ///    TCP Server 使用 - end
    /// ---------------------------------------------


    /// ---------------------------------------------
    ///     TCP Client 使用 - begin
    /// ---------------------------------------------

    /// 连接远端服务器
    DWORD Connect(
            const char *pDstIP,             // 连接目的地址
            WORD wDstPort,                  // 连接目的端口
            bool bAsynCall = false,         // 异步调用connect
            bool bTimeControl = false,      // 是否进行时间控制
            DWORD dwWaitTime = 0            // 等待时间值
            );

    /// 异步调用connect后继续完成连接
    DWORD FinishConnect();

    /// ---------------------------------------------
    ///     TCP Client 使用 - end
    /// ---------------------------------------------


    /// ---------------------------------------------
    ///     TCP 公共使用 - begin
    /// ---------------------------------------------

    /// TCP发送函数
    DWORD Send(
            const void *cpBuf,              // 输入数据缓冲区地址
            DWORD dwBufLen,                 // 输入数据缓冲区的大小
            DWORD *pdwSent = 0,             // 已发送出去的数据
            bool bTimeControl = false,      // 是否进行时间控制
            DWORD dwWaitTime = 0            // 等待时间值
            );

    /// TCP接收数据函数
    DWORD Recv(
            void *pBuf,                     // 输出数据
            DWORD dwBufLen,                 // 输入本缓冲区的大小
            DWORD& rdwRecvLen,              // 输出收到消息的大小
            bool bTimeControl = false,      // 是否进行时间控制
            DWORD dwWaitTime = 0            // 等待时间值
            );

    /// ---------------------------------------------
    ///     TCP 公共使用 - end
    /// ---------------------------------------------


    /// ---------------------------------------------
    ///     UDP 使用 - begin
    /// ---------------------------------------------

    /// UDP发送函数
    DWORD Sendto(
            const void *cpBuf,              // 输入数据缓冲区地址
            DWORD dwBufLen,                 // 输入数据缓冲区的大小
            const char *pDstIP,             // 发送目的地址
            WORD wDstPort                   // 发送目的端口
            );

    /// UDP接收函数
    DWORD Recvfrom(
            void *pBuf,                     // 输出数据
            DWORD dwBufLen,                 // 输入本缓冲区的大小
            DWORD& rdwRecvLen,              // 输出收到消息的大小
            char szSrcIP[OSSOCK_IPSIZE],    // 发送源地址
            WORD &rwSrcPort                 // 发送源端口
            );

    /// ---------------------------------------------
    ///     UDP 使用 - end
    /// ---------------------------------------------

    DWORD Close();
    DWORD Shut();

private:

    /// ---------------------------------------------
    ///     套接字标识
    /// ---------------------------------------------
    /// 未创建      : OSSOCK_NONE
    /// UDP         : OSSOCK_UDP
    /// TCP Server  : OSSOCK_TCPSERVER
    /// TCP Accept  : OSSOCK_TCPACCEPT
    /// TCP Client  : OSSOCK_TCPCLIENT
    /// ---------------------------------------------
    DWORD m_dwSockFlag;                     // 套接字对象标识

    DWORD m_dwHostID;                       // 本机ID

    char m_szHostIP[OSSOCK_IPSIZE];         // 本机IP地址
    WORD m_wHostPort;                       // 本机端口
    WORD m_wReserved;

    /// ---------------------------------------------
    /// 套接字选项
    /// ---------------------------------------------
    int m_iSendBufLen;
    int m_iRecvBufLen;
    int m_iLingerTime;
    int m_iListenNum;
    bool m_bTcpNoDelay;
    bool m_bUdpBind;
    bool m_bUdpBoardcast;

    /// ---------------------------------------------
    /// 未完成发送数据时的重试次数
    /// ---------------------------------------------
    int m_iSendTryTimes;

    struct sockaddr_in m_HostSockAddr;      // 本套接字信息

};


#endif // #ifndef _SOCKTYPE_IF_H_

