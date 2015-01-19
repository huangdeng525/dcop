/// -------------------------------------------------
/// sockLanAppType.cpp : sock应用封装类实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "sockLanAppType.h"
#include <stdio.h>


/*******************************************************
  函 数 名: objLanApp::CreateInstance
  描    述: 创建网络应用
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objLanApp *objLanApp::CreateInstance()
{
    return new CLanAppBase();
}

/*******************************************************
  函 数 名: objLanApp::~objLanApp
  描    述: 析构网络应用
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
objLanApp::~objLanApp()
{
}

/*******************************************************
  函 数 名: LanAppTaskPara::LanAppTaskPara
  描    述: 构造网络应用任务参数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
LanAppTaskPara::LanAppTaskPara(CLanAppBase *pLanApp, CLanAppEventBus *pLanAppEventBus, DWORD dwTaskNo)
{
    if (pLanApp) pLanApp->vIncTaskRef();
    m_pRecvBuf = DCOP_Malloc(OSSOCK_DEFAULT_MTU);
    m_pLanApp = pLanApp;
    m_pLanAppEventBus = pLanAppEventBus;
    m_dwTaskNo = dwTaskNo;
}

/*******************************************************
  函 数 名: LanAppTaskPara::~LanAppTaskPara
  描    述: 析构网络应用任务参数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
LanAppTaskPara::~LanAppTaskPara()
{
    if (m_pLanApp) m_pLanApp->vDecTaskRef();
    if (m_pRecvBuf) DCOP_Free(m_pRecvBuf);
    m_pLanApp = 0;
    m_pLanAppEventBus = 0;
    m_pRecvBuf = 0;
    m_dwTaskNo = 0;
}

/*******************************************************
  函 数 名: LanAppChannel::LanAppChannel
  描    述: 网络应用通道构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
LanAppChannel::LanAppChannel()
{
    m_dwChannelID = 0;
    m_pSock = 0;
    m_pThis = 0;
    m_process = 0;
    (void)memset(m_szRemoteIP, 0, sizeof(m_szRemoteIP));
    m_wRemotePort = 0;

    m_connected = FALSE;
    m_channelType = NONE;

    m_pSavedBuf = 0;
    m_dwSavedBufLen = 0;

    m_pEventBus = 0;
}

/*******************************************************
  函 数 名: LanAppChannel::LanAppChannel
  描    述: 网络应用通道构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
LanAppChannel::LanAppChannel(const LanAppChannel &rChannel)
{
    m_dwChannelID = rChannel.m_dwChannelID;
    m_pSock = rChannel.m_pSock;
    m_pThis = rChannel.m_pThis;
    m_process = rChannel.m_process;
    if (rChannel.m_szRemoteIP)
    {
        (void)strncpy(m_szRemoteIP, rChannel.m_szRemoteIP, sizeof(m_szRemoteIP));
        m_szRemoteIP[sizeof(m_szRemoteIP) - 1] = '\0';
    }
    else
    {
        (void)memset(m_szRemoteIP, 0, sizeof(m_szRemoteIP));
    }
    m_wRemotePort = rChannel.m_wRemotePort;

    m_connected = rChannel.m_connected;
    m_channelType = NONE;

    m_pSavedBuf = 0;
    m_dwSavedBufLen = 0;

    m_pEventBus = 0;
}

/*******************************************************
  函 数 名: LanAppChannel::LanAppChannel
  描    述: 网络应用通道构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
LanAppChannel::LanAppChannel(DWORD dwChannelID,
                        objSock *pSock,
                        CLanAppBase *pThis,
                        const char *szRemoteIP,
                        WORD wRemotePort,
                        BYTE connected)
{
    m_dwChannelID = dwChannelID;
    m_pSock = pSock;
    m_pThis = pThis;
    m_process = 0;
    if (szRemoteIP)
    {
        (void)strncpy(m_szRemoteIP, szRemoteIP, sizeof(m_szRemoteIP));
        m_szRemoteIP[sizeof(m_szRemoteIP) - 1] = '\0';
    }
    else
    {
        (void)memset(m_szRemoteIP, 0, sizeof(m_szRemoteIP));
    }
    m_wRemotePort = wRemotePort;

    m_connected = connected;
    m_channelType = NONE;

    m_pSavedBuf = 0;
    m_dwSavedBufLen = 0;

    m_pEventBus = 0;
}

/*******************************************************
  函 数 名: LanAppChannel::~LanAppChannel
  描    述: 网络应用通道析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
LanAppChannel::~LanAppChannel()
{
    if (!m_pSock)
    {
        return;
    }

    DWORD dwSockFlag = m_pSock->dwGetSockFlag();

    if ((m_channelType == LOCAL) && (dwSockFlag != objSock::OSSOCK_TCPACCEPT))
    {
        /// 需要析构本端队列中自己创建的套接字
        delete m_pSock;
        m_pSock = 0;
    }

    if ((m_channelType == REMOTE) && (dwSockFlag == objSock::OSSOCK_TCPACCEPT))
    {
        /// 需要析构远端队列中接收到的客户套接字
        delete m_pSock;
        m_pSock = 0;
    }

    if (m_pSavedBuf)
    {
        DCOP_Free(m_pSavedBuf);
        m_pSavedBuf = 0;
    }
}

/*******************************************************
  函 数 名: LanAppChannel::operator <
  描    述: <操作符
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool LanAppChannel::operator <(const LanAppChannel &rThis) const
{
    if (m_dwChannelID < rThis.m_dwChannelID)
    {
        return true;
    }

    if (m_pSock && rThis.m_pSock &&
        (m_pSock->sGetSock() < rThis.m_pSock->sGetSock()))
    {
        return true;
    }

    return false;
}

/*******************************************************
  函 数 名: CLanAppEventBus::CLanAppEventBus
  描    述: CLanAppEventBus构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CLanAppEventBus::CLanAppEventBus()
{
    m_pLock = 0;

#if _USE_EPOLL_
    m_epollFd = -1;
#else
    FD_ZERO(&m_fdread);
    FD_ZERO(&m_fdwrite);
#endif
    m_chanEvts = 0;
    m_bAutoExit = true;
}

/*******************************************************
  函 数 名: CLanAppEventBus::~CLanAppEventBus
  描    述: CLanAppEventBus析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CLanAppEventBus::~CLanAppEventBus()
{
#if _USE_EPOLL_
    if (m_epollFd > -1)
    {
        close(m_epollFd);
        m_epollFd = -1;
    }
#endif

    if (m_chanEvts)
    {
        DCOP_Free(m_chanEvts);
        m_chanEvts = 0;
    }

    if (m_pLock)
    {
        m_pLock->Enter();
        delete m_pLock;
        m_pLock = 0;
    }
}

/*******************************************************
  函 数 名: CLanAppEventBus::Init
  描    述: 初始化入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CLanAppEventBus::Init(CLanAppBase *pLanApp)
{
    if (!pLanApp) return FAILURE;

    /// 创建锁
    m_pLock = DCOP_CreateLock();
    if (!m_pLock)
    {
        return FAILURE;
    }

#if _USE_EPOLL_
    /// 创建事件总线(EPOLLCLOEXEC表示执行后关闭，防止子进程复制句柄)
    m_epollFd = epoll_create1(EPOLL_CLOEXEC);
    if (m_epollFd < 0)
    {
        return FAILURE;
    }

    m_chanEvts = (epoll_event *)DCOP_Malloc(sizeof(epoll_event) * EVENT_BUS_SIZE);
    if (!m_chanEvts)
    {
        return FAILURE;
    }

    /// 把初始化之前队列中已添加的句柄制作成事件(因为之前还未初始化事件总线)
    for (IT_EVENTSOCKS it_event = m_eventSocks.begin(); it_event != m_eventSocks.end(); ++it_event)
    {
        AddEvent((LanAppChannel *)(&(*it_event)));
    }
#else
    m_chanEvts = (LanAppChannel **)DCOP_Malloc(sizeof(LanAppChannel *) * EVENT_BUS_SIZE);
    if (!m_chanEvts)
    {
        return FAILURE;
    }
#endif

    DWORD dwTaskCount = 
#if _USE_EPOLL_
    /// epoll模型可以使用多任务作为入口
    pLanApp->GetTaskCount();
#else
    /// select模型只能使用单任务作为入口
    1;
#endif

    for (DWORD i = 0; i < dwTaskCount; ++i)
    {
        /// 创建任务参数
        LanAppTaskPara *pProcessTaskPara = new LanAppTaskPara(pLanApp, this, i);
        if (!pProcessTaskPara)
        {
            return FAILURE;
        }

        /// 创建任务
        char szTaskName[OSNAME_LENGTH];
        (void)snprintf(szTaskName, sizeof(szTaskName), "t%s%u", pLanApp->GetAppName(), (i+1));
        szTaskName[sizeof(szTaskName) - 1] = '\0';
        objTask *pProcessTask = DCOP_CreateTask(szTaskName, 
                            TaskRun, 
                            OSSOCK_DEFAULT_TASK_STACKSIZE, 
                            objTask::OSTASK_PRIORITY_NORMAL, 
                            pProcessTaskPara);
        if (!pProcessTask)
        {
            delete pProcessTaskPara;
            pProcessTaskPara = 0;
            return FAILURE;
        }
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CLanAppEventBus::Add
  描    述: 添加通道
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLanAppEventBus::Add(LanAppChannel *pChannel)
{
    if (!pChannel) return;

    objSock *pSock = pChannel->m_pSock;
    if (!pSock)
    {
        return;
    }

    /// 添加到事件集合中
    {
        AutoLock(m_pLock);

        IT_EVENTSOCKS it_event = m_eventSocks.insert(m_eventSocks.end(), *pChannel);
        if (it_event == m_eventSocks.end())
        {
            return;
        }

        LanAppChannel *pChannelWrite = (LanAppChannel *)(&(*it_event));

        /// c++新标准中，std:set的iterator都是const类型的，
        /// 因为set键值都是不可修改的，会破坏std:set的有序性
        /// 如果确定没有破坏键值，可以强制转换一下

        /// 这里只能把接收到的远程连接的套接字设置为REMOTE (其他的都是本地的，不能删除)
        if (pSock->dwGetSockFlag() == objSock::OSSOCK_TCPACCEPT)
        {
            pChannelWrite->m_channelType = LanAppChannel::REMOTE;
        }

        pChannelWrite->m_pEventBus = this;

    #if _USE_EPOLL_
        AddEvent(pChannelWrite);
    #endif
    }
}

/*******************************************************
  函 数 名: CLanAppEventBus::Del
  描    述: 删除通道
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLanAppEventBus::Del(LanAppChannel *pChannel)
{
    AutoLock(m_pLock);

    (void)m_eventSocks.erase(*pChannel);
}

/*******************************************************
  函 数 名: CLanAppEventBus::TaskRun
  描    述: 任务入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLanAppEventBus::TaskRun(objTask::IPara *para)
{
    LanAppTaskPara *pPara = (LanAppTaskPara *)para;
    OSASSERT(pPara != 0);

    CLanAppEventBus *pThis = pPara->m_pLanAppEventBus;
    OSASSERT(pThis != 0);

    CLanAppBase *pLanApp = pPara->m_pLanApp;
    OSASSERT(pLanApp != 0);

    DWORD dwNullEventCount = 0;

    /// 循环接收事件
    while (!pLanApp->bExit())
    {
    #if !_USE_EPOLL_
        pThis->SetEvents(pLanApp);
    #endif

        /// 如果收到事件数量为正数，进行正常处理
        int iEventCount = pThis->WaitEvents(pLanApp);
        if (iEventCount > 0)
        {
            dwNullEventCount = 0;
            pThis->ProcEvents(pPara, pLanApp, (DWORD)iEventCount);
            continue;
        }

        /// 返回值为负数，可能有异常，先延时一下
        if (iEventCount < 0)
        {
            objTask::Delay(0);
        }

        /// 走到这里是是无事件或者异常返回，如果到一定时间，需要进行退出处理
        if (pThis->bAutoExit())
        {
            if (!pThis->Count()) dwNullEventCount++;
            if (dwNullEventCount > AUTO_EXIT_COUNT)
            {
                delete pThis;
                break;
            }
        }
    }
}

/*******************************************************
  函 数 名: CLanAppEventBus::AddEvent
  描    述: 添加事件
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLanAppEventBus::AddEvent(LanAppChannel *pChannel)
{
#if !_USE_EPOLL_

    return;

#else

    if (!pChannel) return;
    if (m_epollFd <= -1) return;

    objSock *pSock = pChannel->m_pSock;
    if (!pSock)
    {
        return;
    }

    epoll_event epollEv;
    epollEv.data.ptr = pChannel;

    /// 只有TCPClient在未连接前是写事件，其他类型和情况都是读事件
    if ((pSock->dwGetSockFlag() == objSock::OSSOCK_TCPCLIENT) && (pChannel->m_connected != TRUE))
    {
        /// 使用纯异步方式的connect
        (void)pSock->Connect(pChannel->m_szRemoteIP, pChannel->m_wRemotePort, true);
        epollEv.events = EPOLLOUT | EPOLLET;
    }
    else
    {
        epollEv.events = EPOLLIN | EPOLLET;
    }

    (void)epoll_ctl(m_epollFd, EPOLL_CTL_ADD, pSock->sGetSock(), &epollEv);

#endif
}

/*******************************************************
  函 数 名: CLanAppEventBus::SetEvents
  描    述: 设置事件
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLanAppEventBus::SetEvents(CLanAppBase *pLanApp)
{
#if _USE_EPOLL_

    /// epoll只需要添加一次事件
    return;

#else

    if (!pLanApp) return;

    FD_ZERO(&m_fdread);
    FD_ZERO(&m_fdwrite);
    AutoLock(m_pLock);

    for (IT_EVENTSOCKS it_event = m_eventSocks.begin(); it_event != m_eventSocks.end(); ++it_event)
    {
        LanAppChannel *pChannel = (LanAppChannel *)(&(*it_event));
        if (!pChannel)
        {
            continue;
        }

        objSock *pSock = pChannel->m_pSock;
        if (!pSock)
        {
            continue;
        }

        /// 只有TCPClient在未连接前是写事件，其他类型和情况都是读事件
        if ((pSock->dwGetSockFlag() == objSock::OSSOCK_TCPCLIENT) && (pChannel->m_connected != TRUE))
        {
            /// 使用纯异步方式的connect
            (void)pSock->Connect(pChannel->m_szRemoteIP, pChannel->m_wRemotePort, true);
            FD_SET(pSock->sGetSock(), &m_fdwrite);
        }
        else
        {
            FD_SET(pSock->sGetSock(), &m_fdread);
        }
    }

#endif
}

/*******************************************************
  函 数 名: CLanAppEventBus::WaitEvents
  描    述: 等待事件
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
int CLanAppEventBus::WaitEvents(CLanAppBase *pLanApp)
{
    if (!pLanApp) return (-1);

    int iRc;
    DWORD dwBlockTime = OSSOCK_DEFAULT_BLOCKTIME;

#if _USE_EPOLL_

    /// epoll模型直接返回的就是收到事件的句柄列表
    iRc = epoll_wait(m_epollFd, m_chanEvts, EVENT_BUS_SIZE, dwBlockTime);

#else

    struct timeval wait;
    struct timeval *pwait = NULL;

    if (dwBlockTime != OSWAIT_FOREVER)
    {
        wait.tv_sec = (dwBlockTime != 0)? (long)(dwBlockTime / THOUSAND) : 0;
        wait.tv_usec = (dwBlockTime != 0)? (long)((long)(dwBlockTime % THOUSAND) * THOUSAND) : 0;
        pwait = &wait;
    }

    iRc = select(FD_SETSIZE, &m_fdread, &m_fdwrite, NULL, pwait);
    if (iRc <= 0)
    {
        return iRc;
    }

    /// select模型需要遍历所有句柄，才能知道是哪些句柄获得了事件
    {
        AutoLock(m_pLock);
        DWORD dwEventCount = (DWORD)iRc;
        DWORD dwProcCount = 0;

        /// 本端套接字队列
        for (IT_EVENTSOCKS it_event = m_eventSocks.begin(); it_event != m_eventSocks.end(); ++it_event)
        {
            /// 为了不浪费效率，处理完已经接收的事件数量就退出
            if (dwProcCount >= dwEventCount)
            {
                break;
            }

            LanAppChannel *pChannel = (LanAppChannel *)(&(*it_event));
            if (!pChannel)
            {
                continue;
            }

            objSock *pSock = pChannel->m_pSock;
            if (!pSock)
            {
                continue;
            }

            if ((pSock->dwGetSockFlag() == objSock::OSSOCK_TCPCLIENT) && (pChannel->m_connected != TRUE))
            {
                /// TCPClient未连接前是写事件
                if (FD_ISSET(pSock->sGetSock(), &m_fdwrite))
                {
                    m_chanEvts[dwProcCount++] = pChannel;
                }
            }
            else
            {
                /// 其他情况都是读事件
                if (FD_ISSET(pSock->sGetSock(), &m_fdread))
                {
                    m_chanEvts[dwProcCount++] = pChannel;
                }
            }
        }

        iRc = (int)dwProcCount;
    }

#endif

    return iRc;
}

/*******************************************************
  函 数 名: CLanAppEventBus::ProcEvents
  描    述: 处理事件
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLanAppEventBus::ProcEvents(LanAppTaskPara *pPara, CLanAppBase *pLanApp, DWORD dwEventCount)
{
    if (!pLanApp) return;

    for (DWORD i = 0; i < dwEventCount; ++i)
    {
    #if _USE_EPOLL_
        LanAppChannel *pChannel = (LanAppChannel *)m_chanEvts[i].data.ptr;
    #else
        LanAppChannel *pChannel = m_chanEvts[i];
    #endif

        if (!pChannel)
        {
            continue;
        }

        /// 为1是为了保证第一次进入时才能进行事件处理
        DWORD dwRefCount = pChannel->StartProcess();
        if (dwRefCount == 1)
        {
            pLanApp->Process(pPara, pChannel);
        }
    }
}

/*******************************************************
  函 数 名: CLanAppBase::CLanAppBase
  描    述: CLanAppBase构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CLanAppBase::CLanAppBase()
{
    (void)snprintf(m_szAppName, sizeof(m_szAppName), "LanApp");
    m_szAppName[sizeof(m_szAppName) - 1] = '\0';
    m_pLock = 0;
    m_dwLocalID = 0;
    m_dwTaskCount = 1;

    m_fnAcceptProc = 0;
    m_pAcceptProcPara = 0;
    m_fnConnectProc = 0;
    m_pConnectProcPara = 0;
    m_fnDisconnectProc = 0;
    m_pDisconnectProcPara = 0;
    m_fnRecvProc = 0;
    m_pRecvProcPara = 0;

    m_fnBFrameProc = 0;
    m_pBFrameProcPara = 0;
    m_fnChangeBytesOrderProc = 0;
    m_pChangeBytesOrderProcPara = 0;

    m_fnLogPrint = 0;
    m_pLogPrintPara = 0;

    m_startFlag = FALSE;
    m_dwTaskRefCount = 0;
    m_pTaskRefCountSem = 0;
}

/*******************************************************
  函 数 名: CLanAppBase::~CLanAppBase
  描    述: CLanAppBase析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CLanAppBase::~CLanAppBase()
{
    /// 用于结束任务
    Stop();

    m_pLock->Enter();
    DWORD dwTaskCount = m_dwTaskRefCount;
    objCounter *pTaskRefCountSem = m_pTaskRefCountSem;
    m_pLock->Leave();

    /// 阻塞直到所有的任务都退出
    if (dwTaskCount && pTaskRefCountSem)
    {
        for (DWORD i = 0; i < dwTaskCount; ++i)
        {
            pTaskRefCountSem->Take();
        }
    }

    /// 释放锁
    if (m_pLock)
    {
        m_pLock->Enter();
        delete m_pLock;
        m_pLock = 0;
    }

    /// 释放计数器
    if (m_pTaskRefCountSem)
    {
        delete m_pTaskRefCountSem;
        m_pTaskRefCountSem = 0;
    }
}

/*******************************************************
  函 数 名: CLanAppBase::Init
  描    述: 初始化
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CLanAppBase::Init(const char *cszAppName, DWORD dwLocalID, DWORD dwTaskCount,
                        LanEventProc *pEventProc,
                        LanFrameProc *pFrameProc,
                        LanLogProc *pLogProc)
{
    /// 已经开始就不能初始化了
    if (m_startFlag)
    {
        return FAILURE;
    }

    /// 创建锁
    m_pLock = DCOP_CreateLock();
    if (!m_pLock)
    {
        return FAILURE;
    }

    /// 进入锁，然后进行初始化
    {
        AutoLock(m_pLock);

        if (cszAppName && *cszAppName)
        {
            (void)snprintf(m_szAppName, sizeof(m_szAppName), "%s", cszAppName);
            m_szAppName[sizeof(m_szAppName) - 1] = '\0';
        }

        m_dwLocalID = dwLocalID;
        m_dwTaskCount = dwTaskCount;

        if (pEventProc)
        {
            m_fnAcceptProc = pEventProc->fnAcceptProc;
            m_pAcceptProcPara = pEventProc->pAcceptProcPara;
            m_fnConnectProc = pEventProc->fnConnectProc;
            m_pConnectProcPara = pEventProc->pConnectProcPara;
            m_fnDisconnectProc = pEventProc->fnDisconnectProc;
            m_pDisconnectProcPara = pEventProc->pDisconnectProcPara;
            m_fnRecvProc = pEventProc->fnRecvProc;
            m_pRecvProcPara = pEventProc->pRecvProcPara;
        }

        if (pFrameProc)
        {
            m_fnBFrameProc = pFrameProc->fnBFrameProc;
            m_pBFrameProcPara = pFrameProc->pBFrameProcPara;
            m_fnChangeBytesOrderProc = pFrameProc->fnChangeBytesOrderProc;
            m_pChangeBytesOrderProcPara = pFrameProc->pChangeBytesOrderProcPara;
        }

        if (pLogProc)
        {
            m_fnLogPrint = pLogProc->fnLogProc;
            m_pLogPrintPara = pLogProc->pLogProcPara;
        }

        /// 设置启动标志
        (void)objAtomic::Set(m_startFlag, TRUE);
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CLanAppBase::AddChannel
  描    述: 添加通道
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLanAppBase::AddChannel(DWORD dwChannelID,
                        objSock *pSock,
                        const char *szRemoteIP,
                        WORD wRemotePort)
{
    AutoLock(m_pLock);

    LanAppChannel channel(dwChannelID, pSock, this, szRemoteIP, wRemotePort);

    IT_LOCALSOCKS it_local = m_localSocks.insert(m_localSocks.end(), 
                        MAP_LOCALSOCKS::value_type(dwChannelID, channel));
    if (it_local == m_localSocks.end())
    {
        /// 本函数没有返回值，
        /// 外面创建就传进来了，
        /// 只有在本函数释放
        delete pSock;
        pSock = 0;

        return;
    }

    /// 加入到事件总线中
    AddToEventBus(&((*it_local).second));

    ((*it_local).second).m_channelType = LanAppChannel::LOCAL;
}

/*******************************************************
  函 数 名: CLanAppBase::AddTCPServer
  描    述: 添加TCPServer通道
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CLanAppBase::AddTCPServer(DWORD dwChannelID,
                        WORD wLocalPort)
{
    objSock *pSock = DCOP_CreateSock(objSock::OSSOCK_TCPSERVER, 0, wLocalPort);
    if (!pSock)
    {
        return FAILURE;
    }

    if (pSock->Open() != SUCCESS)
    {
        delete pSock;
        return FAILURE;
    }

    AddChannel(dwChannelID, pSock);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CLanAppBase::AddTCPClient
  描    述: 添加TCPClient通道
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CLanAppBase::AddTCPClient(DWORD dwChannelID,
                        const char *szRemoteIP,
                        WORD wRemotePort)
{
    objSock *pSock = DCOP_CreateSock(objSock::OSSOCK_TCPCLIENT, 0, 0);
    if (!pSock)
    {
        return FAILURE;
    }

    if (pSock->Open() != SUCCESS)
    {
        delete pSock;
        return FAILURE;
    }

    AddChannel(dwChannelID, pSock, szRemoteIP, wRemotePort);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CLanAppBase::AddUDP
  描    述: 添加UDP通道
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CLanAppBase::AddUDP(DWORD dwChannelID,
                        bool bBind,
                        WORD wLocalPort,
                        bool bBoardcast)
{
    objSock *pSock = DCOP_CreateSock(objSock::OSSOCK_UDP, 0, ((bBind)? wLocalPort : 0));
    if (!pSock)
    {
        return FAILURE;
    }

    pSock->vSetUdpBind(bBind);
    pSock->vSetUdpBoardcast(bBoardcast);

    if (pSock->Open() != SUCCESS)
    {
        delete pSock;
        return FAILURE;
    }

    AddChannel(dwChannelID, pSock);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CLanAppBase::DelChannel
  描    述: 删除通道
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLanAppBase::DelChannel(DWORD dwChannelID)
{
    AutoLock(m_pLock);

    IT_LOCALSOCKS it_local = m_localSocks.find(dwChannelID);
    if (it_local == m_localSocks.end())
    {
        return;
    }

    DelFromEventBus(&((*it_local).second));

    (void)m_localSocks.erase(dwChannelID);
}

/*******************************************************
  函 数 名: CLanAppBase::Start
  描    述: 开始应用
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CLanAppBase::Start(const char *cszAppName,
                        DWORD dwLocalID,
                        DWORD dwTaskCount,
                        LanEventProc *pEventProc,
                        LanFrameProc *pFrameProc,
                        LanLogProc *pLogProc)
{
    /// 初始化本对象
    DWORD dwRc = Init(cszAppName, dwLocalID, dwTaskCount, 
                        pEventProc, 
                        pFrameProc, 
                        pLogProc);
    if (dwRc != SUCCESS)
    {
        logPrint("LanApp Init Fail!");
        return dwRc;
    }

    /// 初始化第一组事件总线
    dwRc = m_firstEvents.Init(this);
    if (dwRc != SUCCESS)
    {
        logPrint("LanApp Frist Event Bus Init Fail!");
        return dwRc;
    }

    m_firstEvents.vDisableAutoExit();
    return SUCCESS;
}

/*******************************************************
  函 数 名: CLanAppBase::Process
  描    述: 处理
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLanAppBase::Process(LanAppTaskPara *pPara, LanAppChannel *pChannel)
{
    if (!pChannel) return;

    objSock *pSock = pChannel->m_pSock;
    if (!pSock)
    {
        return;
    }

    DWORD dwSockFlag = pSock->dwGetSockFlag();
    if (dwSockFlag == objSock::OSSOCK_TCPSERVER)
    {
        /// TCPServer主机套接字是收到连接事件
        Accept(pPara, pChannel);
    }
    else if (dwSockFlag == objSock::OSSOCK_TCPACCEPT)
    {
        /// TCPServer客户套接字是收到数据事件
        TCPRecv(pPara, pChannel);
    }
    else if (dwSockFlag == objSock::OSSOCK_TCPCLIENT)
    {
        /// TCPClient未连接前是去连接事件，连接后是收数据事件
        if (pChannel->m_connected != TRUE)
        {
            Connect(pPara, pChannel);
        }
        else
        {
            TCPRecv(pPara, pChannel);
        }
    }
    else
    {
        /// 其他情况都是UDP套接字的收数据事件
        UDPRecv(pPara, pChannel);
    }
}

/*******************************************************
  函 数 名: CLanAppBase::Accept
  描    述: 接收连接
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLanAppBase::Accept(LanAppTaskPara *pPara, LanAppChannel *pChannel)
{
    if (!pPara || !pChannel) return;

    objSock *pSock = pChannel->m_pSock;
    if (!pSock)
    {
        return;
    }

    for (;;)
    {
        objSock *pCltSock = 0;
        DWORD dwRc = pSock->Accept(pCltSock);
        if (dwRc != SUCCESS)
        {
            DWORD dwRefCount = pChannel->EndProcess();
            if (dwRefCount)
            {
                continue;
            }

            break;
        }

        dwRc = OnAccept(pChannel->m_dwChannelID, pSock, pCltSock);
        if (dwRc != SUCCESS)
        {
            /// 拒绝请求
            if (pCltSock) delete pCltSock;
            continue;
        }

        /// 加入到事件总线中
        LanAppChannel channel(pChannel->m_dwChannelID, pCltSock, this, 
                            pCltSock->cszGetHostIP(), pCltSock->wGetHostPort(), TRUE);
        AddToEventBus(&channel);
    }
}

/*******************************************************
  函 数 名: CLanAppBase::Connect
  描    述: 连接
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLanAppBase::Connect(LanAppTaskPara *pPara, LanAppChannel *pChannel)
{
    if (!pPara || !pChannel) return;

    objSock *pSock = pChannel->m_pSock;
    if (!pSock)
    {
        return;
    }

    DWORD dwRc = pSock->FinishConnect();
    if (dwRc != SUCCESS)
    {
        return;
    }

    dwRc = OnConnect(pChannel->m_dwChannelID, pSock, 
                        pChannel->m_szRemoteIP, 
                        pChannel->m_wRemotePort);
    pChannel->ClearProcess();
    if (dwRc != SUCCESS)
    {
        /// 拒绝请求，需要重新打开
        (void)pSock->Shut();
        (void)pSock->Close();
        (void)pSock->Open();
        return;
    }

    /// 更新本端队列中的状态
    pChannel->m_connected = TRUE;
}

/*******************************************************
  函 数 名: CLanAppBase::Disconnect
  描    述: 断开连接
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLanAppBase::Disconnect(LanAppTaskPara *pPara, LanAppChannel *pChannel)
{
    if (!pPara || !pChannel) return;

    const char *cszRemoteIP;
    WORD wRemotePort;

    objSock *pSock = pChannel->m_pSock;
    if (!pSock)
    {
        return;
    }

    DWORD dwSockFlag = pSock->dwGetSockFlag();
    if (dwSockFlag == objSock::OSSOCK_TCPCLIENT)
    {
        cszRemoteIP = pChannel->m_szRemoteIP;
        wRemotePort = pChannel->m_wRemotePort;
    }
    else
    {
        cszRemoteIP = pSock->cszGetHostIP();
        wRemotePort = pSock->wGetHostPort();
    }

    OnDisconnect(pChannel->m_dwChannelID, pSock, cszRemoteIP, wRemotePort);

    /// '连接状态'置为FALSE
    pChannel->m_connected = FALSE;

    /// TCPClient需要重新刷新一下套接字
    if (dwSockFlag == objSock::OSSOCK_TCPCLIENT)
    {
        (void)pSock->Shut();
        (void)pSock->Close();
        (void)pSock->Open();
        return;
    }

    /// 其他类型的套接字需要从事件总线中删除，删除后的通道不能再被使用
    DelFromEventBus(pChannel);
}

/*******************************************************
  函 数 名: CLanAppBase::TCPRecv
  描    述: TCP接收数据
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLanAppBase::TCPRecv(LanAppTaskPara *pPara, LanAppChannel *pChannel)
{
    if (!pPara || !pChannel) return;

    objSock *pSock = pChannel->m_pSock;
    if (!pSock)
    {
        return;
    }

    void *pRecvBuf = pPara->m_pRecvBuf;
    if (!pRecvBuf)
    {
        return;
    }

    for (;;)
    {
        DWORD dwRecvLen = 0;
        DWORD dwRc = pSock->Recv(pRecvBuf, OSSOCK_DEFAULT_MTU, dwRecvLen);
        if (dwRc != SUCCESS)
        {
            if (dwRc == ERRCODE_SOCK_CONN_CLSOED)
            {
                /// 连接被关闭
                Disconnect(pPara, pChannel);
                break;
            }

            DWORD dwRefCount = pChannel->EndProcess();
            if (dwRefCount)
            {
                continue;
            }

            break;
        }

        void *pFrameBuf = 0;
        DWORD dwFrameLen = 0;

        /// 支持分帧，则先进行数据缓存
        void *pSavedBuf = pChannel->m_pSavedBuf;
        DWORD dwSavedLen = pChannel->m_dwSavedBufLen;
        if (pSavedBuf)
        {
            if (!(pFrameBuf = DCOP_Malloc(dwSavedLen + dwRecvLen)))
            {
                continue;
            }

            /// 把上次未接收完的数据和这次要接收的数据合并
            (void)memcpy(pFrameBuf, pSavedBuf, dwSavedLen);
            dwFrameLen = dwSavedLen;
            (void)memcpy((BYTE *)pFrameBuf + dwFrameLen, pRecvBuf, dwRecvLen);
            dwFrameLen += dwRecvLen;

            pChannel->m_pSavedBuf = 0;
            pChannel->m_dwSavedBufLen = 0;
            DCOP_Free(pSavedBuf);
        }
        else
        {
            /// 要接收的数据包就是当前收到的
            pFrameBuf = pRecvBuf;
            dwFrameLen = dwRecvLen;
        }

        /// 进行流方式接收数据帧
        pSavedBuf = pGetFrameFromStream(pChannel, pRecvBuf, dwRecvLen, dwSavedLen);
        if (pSavedBuf)
        {
            /// 有余留内存没有收完
            void *dwBufTmp = 0;
            if ((dwBufTmp = DCOP_Malloc(dwSavedLen)) != NULL)
            {
                (void)memcpy(dwBufTmp, pSavedBuf, dwSavedLen);
                pSavedBuf = dwBufTmp;
                pChannel->m_pSavedBuf = pSavedBuf;
                pChannel->m_dwSavedBufLen = dwSavedLen;
            }
        }
    }

    ////////////////////////////////////////////////////
    /// 对于pRecvBuf, 每次接收时固定从开始接收，不用处理
    ////////////////////////////////////////////////////
}

/*******************************************************
  函 数 名: CLanAppBase::UDPRecv
  描    述: UDP接收数据
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLanAppBase::UDPRecv(LanAppTaskPara *pPara, LanAppChannel *pChannel)
{
    if (!pPara || !pChannel) return;

    objSock *pSock = pChannel->m_pSock;
    if (!pSock)
    {
        return;
    }

    void *pRecvBuf = pPara->m_pRecvBuf;
    if (!pRecvBuf)
    {
        return;
    }

    DWORD dwRecvLen = 0;
    DWORD dwRc = pSock->Recvfrom(pRecvBuf, OSSOCK_DEFAULT_MTU, dwRecvLen, 
                        pChannel->m_szRemoteIP, 
                        pChannel->m_wRemotePort);
    pChannel->ClearProcess();
    if (dwRc != SUCCESS)
    {
        return;
    }

    int frameLen = bFrame(pRecvBuf, dwRecvLen);
    if (frameLen <= 0)
    {
        return;
    }

    ChangeBytesOrder(pRecvBuf, (DWORD)frameLen);
    OnRecv(pChannel->m_dwChannelID, pSock, pRecvBuf, (DWORD)frameLen, 
                        pChannel->m_szRemoteIP, 
                        pChannel->m_wRemotePort);
}

/*******************************************************
  函 数 名: CLanAppBase::pGetFrameFromStream
  描    述: 从数据流中接收一个数据帧
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void *CLanAppBase::pGetFrameFromStream(LanAppChannel *pChannel, 
                        void *pRecvBuf, 
                        DWORD dwRecvLen, 
                        DWORD& rdwLeftLen)
{
    if (!pRecvBuf || !dwRecvLen)
    {
        return 0;
    }

    const char *cszRemoteIP;
    WORD wRemotePort;

    objSock *pSock = pChannel->m_pSock;
    if (!pSock)
    {
        return 0;
    }

    DWORD dwSockFlag = pSock->dwGetSockFlag();
    if (dwSockFlag == objSock::OSSOCK_TCPCLIENT)
    {
        cszRemoteIP = pChannel->m_szRemoteIP;
        wRemotePort = pChannel->m_wRemotePort;
    }
    else
    {
        cszRemoteIP = pSock->cszGetHostIP();
        wRemotePort = pSock->wGetHostPort();
    }

    DWORD dwProcLen = 0;
    while (dwProcLen < dwRecvLen)
    {
        void *pBuf = (BYTE *)pRecvBuf + dwProcLen;
        DWORD dwLen = dwRecvLen - dwProcLen;

        ////////////////////////////////////////////////////
        /// 判断是不是一帧的开头
        ////////////////////////////////////////////////////
        int frameLen = bFrame(pBuf, dwLen);
        if (frameLen < 0)
        {
            /// 长度不够判断
            break;
        }

        if (frameLen == 0)
        {
            /// 不是帧头, 只能单字节偏移，重新定位帧头
            dwProcLen++;
            continue;
        }

        if ((DWORD)frameLen > dwLen)
        {
            /// 长度不够一帧
            break;
        }

        ////////////////////////////////////////////////////
        /// 分帧分发消息
        ////////////////////////////////////////////////////
        ChangeBytesOrder(pBuf, (DWORD)frameLen);
        OnRecv(pChannel->m_dwChannelID, pSock, pBuf, (DWORD)frameLen, cszRemoteIP, wRemotePort);
        dwProcLen += (DWORD)frameLen;
    }

    if (dwProcLen >= dwRecvLen)
    {
        return 0;
    }

    rdwLeftLen = dwRecvLen - dwProcLen;
    return (BYTE *)pRecvBuf + dwProcLen;
}

/*******************************************************
  函 数 名: CLanAppBase::AddToEventBus
  描    述: 添加通道到事件总线中
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLanAppBase::AddToEventBus(LanAppChannel *pChannel)
{
    if (!pChannel) return;

    /// 先添加到第一个事件总线中(epoll模型只使用一个事件总线)
#if !_USE_EPOLL_
    if (m_firstEvents.Count() < CLanAppEventBus::EVENT_BUS_SIZE)
#endif
    {
        m_firstEvents.Add(pChannel);
        return;
    }

    LanAppEventGroup eventGroup;

    /// 第一个满了，后面只有添加到扩展分组总线中，而分组是按数量排序的，第一个就是数量最小的
    IT_EVENTGROUPS it_group = m_eventGroups.begin();
    if ((it_group != m_eventGroups.end()) && 
        ((*it_group).m_pEventBus) && 
        ((*it_group).m_pEventBus->Count() < CLanAppEventBus::EVENT_BUS_SIZE))
    {
        eventGroup.m_pEventBus = (*it_group).m_pEventBus;
        (void)m_eventGroups.erase(it_group);
    }
    else
    {
        eventGroup.m_pEventBus = new CLanAppEventBus();
        DWORD dwRc = eventGroup.m_pEventBus->Init(this);
        if (dwRc != SUCCESS)
        {
            CHECK_RETCODE(dwRc, "CLanAppEventBus::Init");
            return;
        }
    }

    eventGroup.m_pEventBus->Add(pChannel);
    (void)m_eventGroups.insert(eventGroup);
}

/*******************************************************
  函 数 名: CLanAppBase::DelFromEventBus
  描    述: 从事件总线中删除通道
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CLanAppBase::DelFromEventBus(LanAppChannel *pChannel)
{
    if (!pChannel) return;

    CLanAppEventBus *pEventBus = pChannel->m_pEventBus;
    if (!pEventBus)
    {
        return;
    }

    /// 如果是第一组，则在第一组中删除中
    if (pEventBus == &m_firstEvents)
    {
        m_firstEvents.Del(pChannel);
        return;
    }

    LanAppEventGroup eventGroup = {pEventBus};

    /// 从扩展分组中查找事件总线，找到后必须从扩展分组中脱离，删除后需要重新加入
    IT_EVENTGROUPS it_group = m_eventGroups.find(eventGroup);
    if (it_group != m_eventGroups.end())
    {
        (void)m_eventGroups.erase(it_group);
    }

    eventGroup.m_pEventBus->Del(pChannel);
    (void)m_eventGroups.insert(eventGroup);
}

