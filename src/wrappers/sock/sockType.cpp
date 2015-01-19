/// -------------------------------------------------
/// sockType.cpp : socket封装类实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "sockType.h"

#if DCOP_OS == DCOP_OS_WINDOWS
#pragma comment(lib, "ws2_32")
#endif


/*******************************************************
  函 数 名: objSock::CreateInstance
  描    述: 创建sock
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objSock *objSock::CreateInstance(DWORD dwSockFlag, const char *cpszIP, WORD wPort, 
                        const char *file, int line)
{
    #undef new
    CSockBase *pSockBase =  new (file, line) CSockBase();
    #define new new(__FILE__, __LINE__)
    if (pSockBase)
    {
        pSockBase->vSetSockFlag(dwSockFlag);
        pSockBase->vSetHostIP(cpszIP);
        pSockBase->vSetHostPort(wPort);
    }

    return pSockBase;
}

/*******************************************************
  函 数 名: objSock::~objSock
  描    述: sock析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objSock::~objSock()
{
}

/*******************************************************
  函 数 名: objSock::GetIPStringByName
  描    述: 获取IP
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void objSock::GetIPStringByName(const char *cszName, char szIP[OSSOCK_IPSIZE])
{
    struct hostent *host = gethostbyname(cszName);

    (void)strncpy(szIP, (char *)inet_ntoa(*(struct in_addr *)(host->h_addr)), OSSOCK_IPSIZE);

    szIP[OSSOCK_IPSIZE - 1] = '\0';
}

/*******************************************************
  函 数 名: objSock::GetIPValueByName
  描    述: 获取IP
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD objSock::GetIPValueByName(const char *cszName)
{
    struct hostent *host = gethostbyname(cszName);

    return (*(struct in_addr *)(host->h_addr)).s_addr;
}

/*******************************************************
  函 数 名: objSock::GetIPValueByString
  描    述: IP字符串转值
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD objSock::GetIPValueByString(const char *cszIP)
{
    return inet_addr((char *)cszIP);
}

/*******************************************************
  函 数 名: objSock::GetIPStringByValue
  描    述: IP值转字符串
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void objSock::GetIPStringByValue(DWORD dwIP, char szIP[OSSOCK_IPSIZE])
{
    struct in_addr in;
    in.s_addr = dwIP;

    (void)strncpy(szIP, (char *)inet_ntoa(in), OSSOCK_IPSIZE);

    szIP[OSSOCK_IPSIZE - 1] = '\0';
}

/*******************************************************
  函 数 名: CSockBase::CSockBase
  描    述: CSockBase构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CSockBase::CSockBase()
{
#if DCOP_OS == DCOP_OS_WINDOWS
    static bool dwWinSockInit = false;
    if (!dwWinSockInit)
    {
        WORD wVersionRequested;
        WSADATA wsaData;
        int err;

        wVersionRequested = MAKEWORD( 2, 2 );

        err = WSAStartup( wVersionRequested, &wsaData );
        if ( err != 0 ) {
            /* Tell the user that we could not find a usable */
            /* WinSock DLL.                                  */
            return;
        }

        /* Confirm that the WinSock DLL supports 2.2.*/
        /* Note that if the DLL supports versions greater    */
        /* than 2.2 in addition to 2.2, it will still return */
        /* 2.2 in wVersion since that is the version we      */
        /* requested.                                        */
         
        if ( LOBYTE( wsaData.wVersion ) != 2 ||
                HIBYTE( wsaData.wVersion ) != 2 ) {
            /* Tell the user that we could not find a usable */
            /* WinSock DLL.                                  */
            WSACleanup( );
            return; 
        }

        /* The WinSock DLL is acceptable. Proceed. */

        dwWinSockInit = true;
    }
#endif

    /// 初始化

    vSetSockFlag(OSSOCK_NONE);
    vSetHostID(0);
    vSetHostIP(0);
    vSetHostPort(OSSOCK_DEFAULT_PORT);
    vSetSendBufLen(OSSOCK_DEFAULT_SENDBUFLEN);
    vSetRecvBufLen(OSSOCK_DEFAULT_RECVBUFLEN);
    vSetLingerTime(0);
    vSetListenNum(OSSOCK_DEFAULT_LISTENNUM);
    vSetTcpNoDelay(false);
    vSetUdpBind(true);
    vSetUdpBoardcast(false);
    vSetSendTryTimes(OSSOCK_DEFAULT_SENDRETRYTIMES);
    vSetSock(INVALID_SOCKET);

    struct sockaddr_in __tmp;
    __tmp.sin_family            = AF_INET;
    __tmp.sin_port              = htons(wGetHostPort());
    __tmp.sin_addr.s_addr       = htonl(INADDR_ANY);
    vSetSockAddr(&__tmp);
}

/*******************************************************
  函 数 名: CSockBase::~CSockBase
  描    述: CSockBase析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CSockBase::~CSockBase()
{
    /// 注: 复制的SOCK类型设置为OSSOCK_NONE，只能用于发送消息
    ///     (在析构时无法自动关闭，但必要时可以调用Shut/Close)

    if (dwGetSockFlag() != OSSOCK_NONE)
    {
        (void)Shut();
        (void)Close();
        vSetSockFlag(OSSOCK_NONE);
    }

    /// windows要求所有的socket退出时使用WSACleanup
    /// 但是这里只是封装一个库，无法知道退出的地方
    /// 所以先不进行退出操作(目前也并没有发现错误)
    /// (构造函数中在第一次初始化时调用了WSAStartup)
}

/*******************************************************
  函 数 名: CSockBase::Open
  描    述: 打开sock
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSockBase::Open()
{
    int iRc = 0;
    DWORD dwRc = SUCCESS;
    int optVal;
    linger lingerOpt;
    SOCKET hostSock = sGetSock();

    struct sockaddr_in __tmp;
    __tmp.sin_family            = AF_INET;
    __tmp.sin_port              = htons(wGetHostPort());
    if (*cszGetHostIP() == '\0')
        __tmp.sin_addr.s_addr   = htonl(INADDR_ANY);
    vSetSockAddr(&__tmp);

    if ((dwGetSockFlag() == OSSOCK_NONE) || (hostSock != INVALID_SOCKET))
    {
        return ERRCODE_SOCK_FLAG_NON_NONE;
    }

    ////////////////////////////////////////////////////
    /// 初始化套接字，并设置不同的选项
    ////////////////////////////////////////////////////

    switch (dwGetSockFlag())
    {
        case OSSOCK_TCPSERVER :
        case OSSOCK_TCPCLIENT :
        {
            hostSock = socket(AF_INET, SOCK_STREAM, 0);
            ERROR_CHECK(hostSock != INVALID_SOCKET);

            /// 设置重复使用选项
            optVal = 1;
            iRc = setsockopt(hostSock,SOL_SOCKET, SO_REUSEADDR, (char *)&optVal, sizeof(optVal));
            ERROR_CHECK(iRc != SOCKET_ERROR);

            /// 设置保持连接选项
            iRc = setsockopt(hostSock, SOL_SOCKET, SO_KEEPALIVE, (char *)&optVal, sizeof(optVal));
            ERROR_CHECK(iRc != SOCKET_ERROR);

            /// 设置延迟选项
            optVal = (bGetTcpNoDelay())? 1 : 0;
            iRc = setsockopt(hostSock, IPPROTO_TCP, TCP_NODELAY, (char *)&optVal, sizeof(optVal));
            ERROR_CHECK(iRc != SOCKET_ERROR);

            /// 设置linger选项
            lingerOpt.l_onoff = 1;
            lingerOpt.l_linger = (u_short)iGetLingerTime();
            iRc = setsockopt(hostSock, SOL_SOCKET, SO_LINGER, (char *)&lingerOpt, sizeof(lingerOpt));
            ERROR_CHECK(iRc != SOCKET_ERROR);
        }
            break;
        case OSSOCK_UDP :
        {
            hostSock = socket(AF_INET, SOCK_DGRAM, 0);

            /// 设置重复使用选项
            optVal = 1;
            iRc = setsockopt(hostSock,SOL_SOCKET, SO_REUSEADDR, (char *)&optVal, sizeof(optVal));
            ERROR_CHECK(iRc != SOCKET_ERROR);

            /// 设置UDP 的广播属性
            if (bGetUdpBoardcast())
            {
                optVal = 1;
                iRc = setsockopt(hostSock, SOL_SOCKET, SO_BROADCAST, (char *)&optVal, sizeof(optVal));
                ERROR_CHECK(iRc != SOCKET_ERROR);
            }
        }
            break;
        default :
            /// 设为空, 不做任何处理(比如TCPServer收到客户端连接的套接字复制)
            return SUCCESS;
    }

    ERROR_CHECK(hostSock > 0);


    ////////////////////////////////////////////////////
    /// 设置所有的套接字公有的选项
    ////////////////////////////////////////////////////

    /// 设置发送缓冲区大小
    optVal = iGetSendBufLen();
    iRc = setsockopt(hostSock, SOL_SOCKET, SO_SNDBUF, (char *)&optVal, sizeof(optVal));
    ERROR_CHECK(iRc != SOCKET_ERROR);

    /// 设置发送缓冲区大小
    optVal = iGetRecvBufLen();
    iRc = setsockopt(hostSock, SOL_SOCKET, SO_RCVBUF, (char *)&optVal, sizeof(optVal));
    ERROR_CHECK(iRc != SOCKET_ERROR);


    ////////////////////////////////////////////////////
    /// 绑定到地址和端口，并设置不同的选项
    ////////////////////////////////////////////////////

    switch (dwGetSockFlag())
    {
        case OSSOCK_TCPSERVER :
        {
            /// 绑定到本地地址和端口
            iRc = bind(hostSock, (struct sockaddr *)cpGetSockAddr(), dwGetSockAddrSize());
            ERROR_CHECK(iRc != SOCKET_ERROR);
 
            iRc = listen(hostSock, iGetListenNum());
            ERROR_CHECK(iRc != SOCKET_ERROR);
        }
            break;
        case OSSOCK_TCPCLIENT :
        {
        }
            break;
        case OSSOCK_UDP :
        {
            if (bGetUdpBind())
            {
                iRc = bind(hostSock, (struct sockaddr *)cpGetSockAddr(), dwGetSockAddrSize());
                ERROR_CHECK(iRc != SOCKET_ERROR);
            }
        }
            break;
        default :
        {
            iRc = SOCKET_ERROR;
            ERROR_CHECK(iRc != SOCKET_ERROR);
        }
            break;
    }

    ERROR_CHECK(dwRc == SUCCESS);

    /// 设置Sock为非阻塞
    (void)setnonblock(hostSock);

    vSetSock(hostSock);
    return SUCCESS;


    ////////////////////////////////////////////////////
    /// 删除过程中失败但是已经创建的套接字
    ////////////////////////////////////////////////////

ERROR_LABEL :

    dwRc = errno;
    if (dwRc == 0)
    {
        dwRc = ERRCODE_SOCK_CREATE_FAILURE;
    }

    if (hostSock > 0)
    {
        close(hostSock);
    }

    return dwRc;
}

/*******************************************************
  函 数 名: CSockBase::Accept
  描    述: TCPServer接收TCPClient连接
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSockBase::Accept(objSock *& rpCltSock, 
                    bool bTimeControl, 
                    DWORD dwWaitTime)
{
    struct timeval wait;
    fd_set fdread;
    fd_set fdexcept;
    int iRc;
    struct sockaddr_in client;
    int iAddrSize;
    SOCKET sClient;
    int optVal;
    SOCKET hostSock = sGetSock();
    rpCltSock = 0;

    if ((dwGetSockFlag() != OSSOCK_TCPSERVER) || (INVALID_SOCKET == hostSock))
    {
        return ERRCODE_SOCK_INVALID_SOCKET;
    }

    if (bTimeControl)
    {
        FD_ZERO(&fdread);
        FD_ZERO(&fdexcept);
        FD_SET(hostSock, &fdread);
        FD_SET(hostSock, &fdexcept);

        struct timeval *pwait = NULL;
        if (dwWaitTime != OSWAIT_FOREVER)
        {
            wait.tv_sec = (dwWaitTime != 0)? (long)(dwWaitTime / THOUSAND) : 0;
            wait.tv_usec = (dwWaitTime != 0)? (long)((long)(dwWaitTime % THOUSAND) * THOUSAND) : 0;
            pwait = &wait;
        }

        iRc = select(hostSock + 1, &fdread, NULL, &fdexcept, pwait);
        ERROR_CHECK(iRc >= 0);
        if (iRc == 0)
        {
            return ERRCODE_SOCK_SELECT_TIMEOUT;
        }

        /// select返回值是收到的事件的socket个数
        if (FD_ISSET(hostSock, &fdread))
        {
            bTimeControl = false; // 经过这里才会进行下面的操作
        }
        else
        {
            if (FD_ISSET(hostSock, &fdexcept))
            {
                return ERRCODE_SOCK_CONN_CLSOED;
            }

            ERROR_CHECK(false); // 一定走到ERROR_LABEL标记
        }
    }

    if (!bTimeControl)
    {
        iAddrSize = sizeof(client);
        sClient = accept(hostSock, (struct sockaddr *)&client, (socklen_t *)&iAddrSize);
        ERROR_CHECK(sClient != INVALID_SOCKET);

        rpCltSock = DCOP_CreateSock(OSSOCK_NONE,
                        (char *)inet_ntoa(client.sin_addr),
                        (WORD)ntohs(client.sin_port));
        if (!rpCltSock)
        {
            close(sClient);
            return FAILURE;
        }

        /// 实际上是在服务端收到的客户端连接
        rpCltSock->vSetSockFlag(OSSOCK_TCPACCEPT);
        rpCltSock->vSetSock(sClient);
        rpCltSock->vSetSockAddr(&client);

        /// 设置发送缓冲区大小
        optVal = iGetSendBufLen();
        iRc = setsockopt(sClient, SOL_SOCKET, SO_SNDBUF, (char *)&optVal, sizeof(optVal));
        ERROR_CHECK(iRc != SOCKET_ERROR);

        /// 设置发送缓冲区大小
        optVal = iGetRecvBufLen();
        iRc = setsockopt(sClient, SOL_SOCKET, SO_RCVBUF, (char *)&optVal, sizeof(optVal));
        ERROR_CHECK(iRc != SOCKET_ERROR);

        /// 设置客户套接字为非阻塞
        iRc = setnonblock(sClient);
        ERROR_CHECK(iRc != SOCKET_ERROR);

        return SUCCESS;

    }

ERROR_LABEL :

    DWORD dwRc = errno;
    if (dwRc == 0)
    {
        dwRc = ERRCODE_SOCK_ACCEPT_ERROR;
    }

    if (rpCltSock)
    {
        delete rpCltSock;
        rpCltSock = 0;
    }

    return dwRc;
}

/*******************************************************
  函 数 名: CSockBase::Connect
  描    述: TCPClient连接TCPServer
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSockBase::Connect(const char *pDstIP, 
                    WORD wDstPort, 
                    bool bAsynCall, 
                    bool bTimeControl, 
                    DWORD dwWaitTime)
{
    struct timeval wait;
    fd_set fdwrite;
    int iRc = 0;
    struct sockaddr_in dstAddr;
    DWORD dwDstAddr;
    int optVal;
    SOCKET hostSock = sGetSock();

    if ((dwGetSockFlag() != OSSOCK_TCPCLIENT) || (INVALID_SOCKET == hostSock))
    {
        return ERRCODE_SOCK_INVALID_SOCKET;
    }

    dwDstAddr = inet_addr((char *)pDstIP);
    dstAddr.sin_family          = AF_INET;
    dstAddr.sin_port            = htons(wDstPort);
    dstAddr.sin_addr.s_addr     = dwDstAddr;

    /// 异步和时间控制模式下，先设置connect为非阻塞连接
    if (bAsynCall || bTimeControl)
    {
        iRc = setnonblock(hostSock);
        ERROR_CHECK(iRc != SOCKET_ERROR);
    }

    iRc = connect(hostSock, (struct sockaddr *)&dstAddr, sizeof(struct sockaddr));

    /// 异步调用时先返回成功，后面需要FinishConnect
    if (bAsynCall) iRc = SUCCESS;

    /// 失败时如果要时间控制的话则需要进行状态等待
    if ((iRc == SOCKET_ERROR) && bTimeControl)
    {
        FD_ZERO(&fdwrite);
        FD_SET(hostSock, &fdwrite);

        if (dwWaitTime != OSWAIT_FOREVER)
        {
            wait.tv_sec = (dwWaitTime != 0)? (long)(dwWaitTime / THOUSAND) : 0;
            wait.tv_usec = (dwWaitTime != 0)? (long)((long)(dwWaitTime % THOUSAND) * THOUSAND) : 0;
            iRc = select(hostSock + 1, NULL, &fdwrite, NULL, &wait);
            ERROR_CHECK(iRc >= 0);
        }
        else
        {
            iRc = select(hostSock + 1, NULL, &fdwrite, NULL, NULL);
            ERROR_CHECK(iRc >= 0);
        }

        if (iRc == 0)
        {
            return ERRCODE_SOCK_SELECT_TIMEOUT;
        }

        /// 使用getsockopt进行判断，不用使用ISSET，更为准确
        optVal = sizeof(int);
        getsockopt(hostSock, SOL_SOCKET, SO_ERROR, (char *)&iRc, (socklen_t *)&optVal);
        if (iRc != 0) iRc = SOCKET_ERROR;
    }

    /// 再一起判断前面的操作是否失败
    ERROR_CHECK(iRc != SOCKET_ERROR);

    /// 设置Sock为非阻塞
    (void)setnonblock(hostSock);

    return SUCCESS;

ERROR_LABEL :

    DWORD dwRc = errno;
    if (dwRc == 0)
    {
        dwRc = ERRCODE_SOCK_CONN_ERROR;
    }

    return dwRc;
}

/*******************************************************
  函 数 名: CSockBase::FinishConnect
  描    述: 异步调用connect后继续完成连接
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSockBase::FinishConnect()
{
    int iRc = 0;
    SOCKET hostSock = sGetSock();

    if ((dwGetSockFlag() != OSSOCK_TCPCLIENT) ||
        (INVALID_SOCKET == hostSock))
    {
        return ERRCODE_SOCK_INVALID_SOCKET;
    }

    /// 使用getsockopt进行判断，不用使用ISSET，更为准确
    int optVal = sizeof(int);
    getsockopt(hostSock, SOL_SOCKET, SO_ERROR, (char *)&iRc, (socklen_t *)&optVal);
    if (iRc != 0)
    {
        DWORD dwRc = errno;
        if (dwRc == 0)
        {
            dwRc = ERRCODE_SOCK_CONN_ERROR;
        }

        return dwRc;
    }

    /// 设置Sock为非阻塞
    (void)setnonblock(hostSock);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CSockBase::Send
  描    述: TCP发送
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSockBase::Send(const void *cpBuf, 
                    DWORD dwBufLen, 
                    DWORD *pdwSent, 
                    bool bTimeControl, 
                    DWORD dwWaitTime)
{
    struct timeval wait;
    fd_set fdwrite;
    fd_set fdexcept;
    int iRc;
    DWORD dwHaveSent = 0;
    int iSendTryTimes = 0;
    SOCKET hostSock = sGetSock();

    if (pdwSent) *pdwSent = 0;

    if (!cpBuf || !dwBufLen)
    {
        return ERRCODE_SOCK_PARA_ERR_VALUE;
    }

    if (INVALID_SOCKET == hostSock)
    {
        return ERRCODE_SOCK_INVALID_SOCKET;
    }

    if (bTimeControl && (dwWaitTime != OSWAIT_FOREVER))
    {
        wait.tv_sec = (dwWaitTime != 0)? (long)(dwWaitTime / THOUSAND) : 0;
        wait.tv_usec = (dwWaitTime != 0)? (long)((long)(dwWaitTime % THOUSAND) * THOUSAND) : 0;
    }

    while ((dwHaveSent < dwBufLen) && (iSendTryTimes <= iGetSendTryTimes()))
    {
        if (bTimeControl)
        {
            FD_ZERO(&fdwrite);
            FD_ZERO(&fdexcept);
            FD_SET(hostSock, &fdwrite);
            FD_SET(hostSock, &fdexcept);

            struct timeval *pwait = NULL;
            if (dwWaitTime != OSWAIT_FOREVER)
            {
                pwait = &wait;
            }

            iRc = select(hostSock + 1, NULL, &fdwrite, &fdexcept, pwait);
            if (iRc < 0) break;         // 出错
            if (iRc == 0) continue;     // 超时
            if (!FD_ISSET(hostSock, &fdwrite))
            {
                if (FD_ISSET(hostSock, &fdexcept))
                {
                    break;              // 异常事件
                }

                continue;               // 其他事件
            }

            /// 能继续走下去的，都是收到了写事件，可以进行Send
        }

        iRc = send(hostSock, 
                    (char *)((BYTE *)cpBuf + dwHaveSent), 
                    (int)(dwBufLen - dwHaveSent), 
                    MSG_NOSIGNAL);
        if (iRc > 0)
        {
            dwHaveSent += (DWORD)iRc;
        }

        iSendTryTimes++;
    }

    if (pdwSent) *pdwSent = dwHaveSent;

    if (dwHaveSent >= dwBufLen)
    {
        return SUCCESS;
    }

    DWORD dwRc = errno;
    if (dwRc == 0)
    {
        dwRc = ERRCODE_SOCK_SEND_ERROR;
    }

    return dwRc;
}

/*******************************************************
  函 数 名: CSockBase::Recv
  描    述: TCP接收
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSockBase::Recv(void *pBuf, 
                    DWORD dwBufLen, 
                    DWORD& rdwRecvLen, 
                    bool bTimeControl, 
                    DWORD dwWaitTime)
{
    struct timeval wait;
    fd_set fdread;
    fd_set fdexcept;
    int iRc;
    SOCKET hostSock = sGetSock();

    if (!pBuf || !dwBufLen)
    {
        return ERRCODE_SOCK_PARA_ERR_VALUE;
    }

    if (INVALID_SOCKET == hostSock)
    {
        return ERRCODE_SOCK_INVALID_SOCKET;
    }

    if (bTimeControl)
    {
        FD_ZERO(&fdread);
        FD_ZERO(&fdexcept);
        FD_SET(hostSock, &fdread);
        FD_SET(hostSock, &fdexcept);

        if (dwWaitTime != OSWAIT_FOREVER)
        {
            wait.tv_sec = (dwWaitTime != 0)? (long)(dwWaitTime / THOUSAND) : 0;
            wait.tv_usec = (dwWaitTime != 0)? (long)((long)(dwWaitTime % THOUSAND) * THOUSAND) : 0;
            iRc = select(hostSock + 1, &fdread, NULL, &fdexcept, &wait);
            ERROR_CHECK(iRc >= 0);
        }
        else
        {
            iRc = select(hostSock + 1, &fdread, NULL, &fdexcept, NULL);
            ERROR_CHECK(iRc >= 0);
        }

        if (iRc == 0)
        {
            return ERRCODE_SOCK_SELECT_TIMEOUT;
        }

        /// select返回值是收到的事件的socket个数
        if (FD_ISSET(hostSock, &fdread))
        {
            bTimeControl = false; // 经过这里才会进行下面的操作
        }
        else
        {
            if (FD_ISSET(hostSock, &fdexcept))
            {
                return ERRCODE_SOCK_CONN_CLSOED;
            }

            ERROR_CHECK(false); // 一定走到ERROR_LABEL标记
        }
    }

    if (!bTimeControl)
    {
        if ((dwGetSockFlag() != OSSOCK_TCPACCEPT) &&
            (dwGetSockFlag() != OSSOCK_TCPCLIENT))
        {
            return ERRCODE_SOCK_FLAG_ERR_VALUE;
        }

        iRc = recv(hostSock, (char *)pBuf, (int)dwBufLen, 0);
        if (iRc > 0)
        {
            rdwRecvLen = (DWORD)iRc;
            return SUCCESS;
        }

        if (iRc == 0) // 等于零表示 "graceful closed!", 也一并返回错误
        {
            return ERRCODE_SOCK_CONN_CLSOED;
        }

        ERROR_CHECK(false); // 一定走到ERROR_LABEL标记
    
    }

ERROR_LABEL :

    DWORD dwRc = errno;
    if (dwRc == 0)
    {
        dwRc = ERRCODE_SOCK_RECV_ERROR;
    }

    return dwRc;
}

/*******************************************************
  函 数 名: CSockBase::Sendto
  描    述: UDP发送
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSockBase::Sendto(const void *cpBuf, 
                    DWORD dwBufLen, 
                    const char *pDstIP, 
                    WORD wDstPort
                    )
{
    int iRc = 0;
    struct sockaddr_in dstAddr;
    DWORD dwDstAddr;
    DWORD dwHaveSent = 0;
    int iSendTryTimes = 0;
    SOCKET hostSock = sGetSock();

    if (!cpBuf || !dwBufLen)
    {
        return ERRCODE_SOCK_PARA_ERR_VALUE;
    }

    if ((dwGetSockFlag() != OSSOCK_UDP) ||
        (INVALID_SOCKET == hostSock))
    {
        return ERRCODE_SOCK_INVALID_SOCKET;
    }

    dwDstAddr = inet_addr((char *)pDstIP);
    dstAddr.sin_family          = AF_INET;
    dstAddr.sin_port            = htons(wDstPort);
    dstAddr.sin_addr.s_addr     = dwDstAddr;

    while ((dwHaveSent < dwBufLen) &&
        (iSendTryTimes <= iGetSendTryTimes()))
    {
        iRc = sendto(hostSock, 
                    (char *)((BYTE *)cpBuf + dwHaveSent),
                    (int)(dwBufLen - dwHaveSent),
                    MSG_NOSIGNAL, 
                    (struct sockaddr *)&dstAddr, 
                    sizeof(struct sockaddr));
        if (iRc > 0)
        {
            dwHaveSent += (DWORD)iRc;
        }

        iSendTryTimes++;
    }

    if (dwHaveSent >= dwBufLen)
    {
        return SUCCESS;
    }
    else
    {
        DWORD dwRc = errno;
        if (dwRc == 0)
        {
            dwRc = ERRCODE_SOCK_SEND_ERROR;
        }

        return dwRc;
    }
}

/*******************************************************
  函 数 名: CSockBase::Recvfrom
  描    述: UDP接收
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSockBase::Recvfrom(void *pBuf, 
                    DWORD dwBufLen, 
                    DWORD& rdwRecvLen, 
                    char szSrcIP[OSSOCK_IPSIZE], 
                    WORD &rwSrcPort
                    )
{
    int iRc = 0;
    struct sockaddr_in dstAddr;
    int len;
    SOCKET hostSock = sGetSock();

    if (!pBuf || !dwBufLen)
    {
        return ERRCODE_SOCK_PARA_ERR_VALUE;
    }

    if ((dwGetSockFlag() != OSSOCK_UDP) ||
        (INVALID_SOCKET == hostSock))
    {
        return ERRCODE_SOCK_INVALID_SOCKET;
    }

    len = sizeof(dstAddr);

    iRc = recvfrom(hostSock, 
                    (char *)pBuf, 
                    (int)dwBufLen, 
                    0, 
                    (struct sockaddr *)&dstAddr, 
                    (socklen_t *)&len);
    if (iRc > 0)
    {
        if (szSrcIP)
        {
            (void)strncpy(szSrcIP, (char *)inet_ntoa(dstAddr.sin_addr), OSSOCK_IPSIZE);
            szSrcIP[OSSOCK_IPSIZE - 1] = '\0';
        }
        rwSrcPort = (WORD)ntohs(dstAddr.sin_port);

        rdwRecvLen = (DWORD)iRc;
        return SUCCESS;
    }

    DWORD dwRc = errno;
    if (dwRc == 0)
    {
        dwRc = ERRCODE_SOCK_RECV_ERROR;
    }

    return dwRc;
}

/*******************************************************
  函 数 名: CSockBase::Close
  描    述: 直接关闭
            [断开连接及直接释放句柄]
            [用于接收线程强制关闭]
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSockBase::Close()
{
    SOCKET hostSock = sGetSock();
    if (hostSock == INVALID_SOCKET)
    {
        return ERRCODE_SOCK_INVALID_SOCKET;
    }

    int iRc = close(hostSock);
    if (iRc >= 0)
    {
        vSetSock(INVALID_SOCKET);
        return SUCCESS;
    }

    DWORD dwRc = errno;
    if (dwRc == 0)
    {
        dwRc = FAILURE;
    }

    return dwRc;
}

/*******************************************************
  函 数 名: CSockBase::Shut
  描    述: 优雅关闭
            [切断发送通道，不会释放句柄]
            [用于发送线程触发，紧接着接收线程在Recv接口返回错误时再调用Close关闭]
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CSockBase::Shut()
{
    SOCKET hostSock = sGetSock();
    if (hostSock == INVALID_SOCKET)
    {
        return ERRCODE_SOCK_INVALID_SOCKET;
    }

    /// 第2个参数how==1，是指禁止进行"发送"，同时触发对端套接字进入关闭流程
    int iRc = shutdown(hostSock, 1);
    if (iRc >= 0)
    {
        return SUCCESS;
    }

    DWORD dwRc = errno;
    if (dwRc == 0)
    {
        dwRc = FAILURE;
    }

    return dwRc;
}

