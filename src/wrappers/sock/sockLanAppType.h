/// -------------------------------------------------
/// sockLanAppType.h : sock应用封装类公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _LANAPPTYPE_H_
#define _LANAPPTYPE_H_

#define INC_SET
#define INC_MAP

#include "msg.h"
#include "sem.h"
#include "task.h"
#include "sock.h"
#include "sockType.h"

/// 如果没有专门配置EPOLL是否启用(1)还是不用(0)，linux下默认打开EPOLL
#if !defined(_USE_EPOLL_) && (DCOP_OS == DCOP_OS_LINUX)
#define _USE_EPOLL_ 1
#endif

#if _USE_EPOLL_
#include <sys/epoll.h>
#endif


/// 网络应用
class CLanAppBase;
class CLanAppEventBus;


/// 网络应用任务参数
struct LanAppTaskPara : public objTask::IPara
{
    CLanAppBase *m_pLanApp;
    CLanAppEventBus *m_pLanAppEventBus;
    void *m_pRecvBuf;
    DWORD m_dwTaskNo;

    LanAppTaskPara(CLanAppBase *pLanApp, CLanAppEventBus *pLanAppEventBus, DWORD dwTaskNo);
    ~LanAppTaskPara();
};


/// 网络应用通道
struct LanAppChannel
{
    DWORD m_dwChannelID;
    objSock *m_pSock;
    CLanAppBase *m_pThis;
    objAtomic::T m_process;
    char m_szRemoteIP[OSSOCK_IPSIZE];
    WORD m_wRemotePort;
    BYTE m_connected;
    BYTE m_channelType;
    void *m_pSavedBuf;
    DWORD m_dwSavedBufLen;
    CLanAppEventBus *m_pEventBus;

    enum {NONE, LOCAL, REMOTE};

    LanAppChannel();
    LanAppChannel(const LanAppChannel &rChannel);
    LanAppChannel(DWORD dwChannelID,
                        objSock *pSock,
                        CLanAppBase *pThis,
                        const char *szRemoteIP,
                        WORD wRemotePort,
                        BYTE connected = FALSE);
    ~LanAppChannel();
    bool operator <(const LanAppChannel &rThis) const;
    DWORD StartProcess() {return (DWORD)objAtomic::Inc(m_process);}
    DWORD EndProcess() {return (DWORD)objAtomic::Dec(m_process);}
    void ClearProcess() {(void)objAtomic::Set(m_process, 0);}
};


/// 网络应用事件总线
class CLanAppEventBus
{
public:
    typedef std::set<LanAppChannel> SET_EVENTSOCKS;
    typedef SET_EVENTSOCKS::iterator IT_EVENTSOCKS;

#if _USE_EPOLL_
    /// epoll虽然可以绑定很大数量的句柄，但是一次不能处理太多(最好和多任务配合)
    static const DWORD EVENT_BUS_SIZE = 1;
#else
    /// select最多只支持1024个句柄(需要扩展)，超过后需要扩充
    static const DWORD EVENT_BUS_SIZE = 1024;
#endif

    /// 自动退出时的计数周期
    static const DWORD AUTO_EXIT_COUNT = 16;

public:
    CLanAppEventBus();
    ~CLanAppEventBus();

    DWORD Init(CLanAppBase *pLanApp);

    void Add(LanAppChannel *pChannel);
    void Del(LanAppChannel *pChannel);

    DWORD Count() {AutoLock(m_pLock); return (DWORD)m_eventSocks.size();}

    void vDisableAutoExit() {m_bAutoExit = false;}
    bool bAutoExit() {return m_bAutoExit;}

private:
    static void TaskRun(objTask::IPara *para);
    void AddEvent(LanAppChannel *pChannel);
    void SetEvents(CLanAppBase *pLanApp);
    int  WaitEvents(CLanAppBase *pLanApp);
    void ProcEvents(LanAppTaskPara *pPara, CLanAppBase *pLanApp, DWORD dwEventCount);

private:
    objLock *m_pLock;

#if _USE_EPOLL_
    int m_epollFd;
    epoll_event *m_chanEvts;
#else
    fd_set m_fdread;
    fd_set m_fdwrite;
    LanAppChannel **m_chanEvts;
#endif

    SET_EVENTSOCKS m_eventSocks;
    bool m_bAutoExit;
};


/// 网络应用事件分组
struct LanAppEventGroup
{
    CLanAppEventBus *m_pEventBus;

    bool operator <(const LanAppEventGroup &rThis) const
    {
        if (!m_pEventBus || !rThis.m_pEventBus)
        {
            return false;
        }

        if (m_pEventBus == rThis.m_pEventBus)
        {
            return false;
        }

        if (m_pEventBus->Count() > rThis.m_pEventBus->Count())
        {
            return false;
        }

        return true;
    }
};


/// 网络应用对象
class CLanAppBase : public objLanApp, private osBase
{
public:
    CLanAppBase();
    ~CLanAppBase();

    typedef std::map<DWORD, LanAppChannel> MAP_LOCALSOCKS;
    typedef MAP_LOCALSOCKS::iterator IT_LOCALSOCKS;
    typedef std::set<LanAppEventGroup> SET_EVENTGROUPS;
    typedef SET_EVENTGROUPS::iterator IT_EVENTGROUPS;

    /// 日志打印
#define logPrint(szInfo) \
    logPrintEx(szInfo, __FILE__, __LINE__)
    void logPrintEx(const char *cszLogInfo, 
                        const char *pFile,
                        DWORD dwLine)
    {
        VDBACKFUNC_VDCALL(m_fnLogPrint)(
                        cszLogInfo,
                        pFile,
                        dwLine,
                        m_pLogPrintPara);
    }

    DWORD Init(const char *cszAppName, DWORD dwLocalID, DWORD dwTaskCount,
                        LanEventProc *pEventProc,
                        LanFrameProc *pFrameProc,
                        LanLogProc *pLogProc);

    void AddChannel(DWORD dwChannelID,
                        objSock *pSock,
                        const char *szRemoteIP = 0,
                        WORD wRemotePort = 0);

    const char *GetAppName() {return m_szAppName;}

    DWORD GetTaskCount() {return m_dwTaskCount;}

    void Process(LanAppTaskPara *pPara, LanAppChannel *pChannel);

    void Accept(LanAppTaskPara *pPara, LanAppChannel *pChannel);

    void Connect(LanAppTaskPara *pPara, LanAppChannel *pChannel);

    void Disconnect(LanAppTaskPara *pPara, LanAppChannel *pChannel);

    void TCPRecv(LanAppTaskPara *pPara, LanAppChannel *pChannel);

    void UDPRecv(LanAppTaskPara *pPara, LanAppChannel *pChannel);

    DWORD OnAccept(DWORD dwChannelID, objSock *pServerSock, objSock *pAcceptSock)
    {
        return BACKFUNC_CALL(m_fnAcceptProc)(dwChannelID, pServerSock, pAcceptSock, m_pAcceptProcPara);
    }
    DWORD OnConnect(DWORD dwChannelID, objSock *pClientSock, const char *cszRemoteIP, WORD wRemotePort)
    {
        return BACKFUNC_CALL(m_fnConnectProc)(dwChannelID, pClientSock, cszRemoteIP, wRemotePort, m_pConnectProcPara);
    }
    void OnDisconnect(DWORD dwChannelID, objSock *pSock, const char *cszRemoteIP, WORD wRemotePort)
    {
        VDBACKFUNC_VDCALL(m_fnDisconnectProc)(dwChannelID, pSock, cszRemoteIP, wRemotePort, m_pDisconnectProcPara);
    }
    void OnRecv(DWORD dwChannelID, objSock *pSock, void *pFrameBuf, DWORD dwFrameLen, const char *cszRemoteIP, WORD wRemotePort)
    {
        VDBACKFUNC_VDCALL(m_fnRecvProc)(dwChannelID, pSock, pFrameBuf, dwFrameLen, cszRemoteIP, wRemotePort, m_pRecvProcPara);
    }

    int bFrame(void *pBuf, DWORD dwLen)
    {
        if (m_fnBFrameProc)
        {
            return (m_fnBFrameProc)(pBuf, dwLen, m_pBFrameProcPara);
        }

        return OSMsgHeader::bFrame(pBuf, dwLen);
    }
    void ChangeBytesOrder(void *pBuf, DWORD dwLen)
    {
        if (m_fnChangeBytesOrderProc)
        {
            return (m_fnChangeBytesOrderProc)(pBuf, dwLen, m_pChangeBytesOrderProcPara);
        }

        return OSMsgHeader::ChangeBytesOrder(pBuf, dwLen);
    }
    

    ////////////////////////////////////////////////////
    /// 实现继承的虚接口 - begin
    ////////////////////////////////////////////////////

    /// 获取本端ID
    DWORD GetLocalID() {return m_dwLocalID;}

    /// 获取通道套接字
    objSock *GetChannel(DWORD dwChannelID)
    {
        AutoLock(m_pLock);

        IT_LOCALSOCKS it_local = m_localSocks.find(dwChannelID);
        if (it_local == m_localSocks.end())
        {
            return NULL;
        }

        return ((*it_local).second).m_pSock;
    }

    /// 添加TCPServer通道
    DWORD AddTCPServer(DWORD dwChannelID,
                        WORD wLocalPort);

    /// 添加TCPClient通道
    DWORD AddTCPClient(DWORD dwChannelID,
                        const char *szRemoteIP,
                        WORD wRemotePort);

    /// 添加UDP通道
    DWORD AddUDP(DWORD dwChannelID,
                        bool bBind,
                        WORD wLocalPort,
                        bool bBoardcast);

    /// 删除通道
    void DelChannel(DWORD dwChannelID);

    /// 启动应用
    DWORD Start(const char *cszAppName,
                        DWORD dwLocalID,
                        DWORD dwTaskCount,
                        LanEventProc *pEventProc,
                        LanFrameProc *pFrameProc = 0,
                        LanLogProc *pLogProc = 0);

    /// 停止应用
    void Stop()
    {
        (void)objAtomic::Set(m_startFlag, FALSE);
    }

    ////////////////////////////////////////////////////
    /// 实现继承的虚接口 - end
    ////////////////////////////////////////////////////


public:
    ////////////////////////////////////////////////////
    /// Name  : dwGetFrameFromStream
    /// Desp  : 从流式数据中拆出数据包的边界
    /// Input : pChannel    远端发送ID
    ///       : pRecvBuf    缓冲区首地址
    ///       : dwRecvLen   缓冲区大小
    /// Output: rdwLeftLen  剩下的缓冲区长度
    /// Return: 
    ///         本次接收后, 最后不是一个完整的Frame,返回值指向
    ///         消息包中剩下的缓冲区(只是指向pRecvBuf的某个位置)
    ////////////////////////////////////////////////////
    void *pGetFrameFromStream(LanAppChannel *pChannel, 
                        void *pRecvBuf, 
                        DWORD dwRecvLen, 
                        DWORD& rdwLeftLen);

    /// 判断是否希望结束(任务获取此状态后自行退出)
    bool bExit() {return (!m_startFlag)? true : false;}

    /// 递增任务索引
    void vIncTaskRef()
    {
        AutoLock(m_pLock);

        if (!m_pTaskRefCountSem)
        {
            m_pTaskRefCountSem = DCOP_CreateCounter(0, OSSOCK_LANAPP_TASK_MAX_COUNT);
            if (!m_pTaskRefCountSem) return;
        }

        m_dwTaskRefCount++;
    }

    /// 递减任务索引
    void vDecTaskRef()
    {
        AutoLock(m_pLock);

        if (m_pTaskRefCountSem) m_pTaskRefCountSem->Give();
    }

    /// 操作事件总线中的通道
    void AddToEventBus(LanAppChannel *pChannel);
    void DelFromEventBus(LanAppChannel *pChannel);

private:
    /// 配置参数
    char m_szAppName[OSNAME_LENGTH];
    DWORD m_dwLocalID;
    DWORD m_dwTaskCount;

    /// 事件总线分组
    objLock *m_pLock;
    MAP_LOCALSOCKS m_localSocks;
    CLanAppEventBus m_firstEvents;
    SET_EVENTGROUPS m_eventGroups;

    /// 事件处理回调
    FUNC_ON_ACCEPT m_fnAcceptProc;
    void *m_pAcceptProcPara;
    FUNC_ON_CONNECT m_fnConnectProc;
    void *m_pConnectProcPara;
    FUNC_ON_DISCONNECT m_fnDisconnectProc;
    void *m_pDisconnectProcPara;
    FUNC_ON_RECV m_fnRecvProc;
    void *m_pRecvProcPara;

    /// 数据帧处理回调
    FUNC_B_FRAME m_fnBFrameProc;
    void *m_pBFrameProcPara;
    FUNC_CHANGE_BYTES_ORDER m_fnChangeBytesOrderProc;
    void *m_pChangeBytesOrderProcPara;

    /// 日志回调
    FUNC_LOG_PRINT m_fnLogPrint;
    void *m_pLogPrintPara;

    /// 开始标识-FALSE:未开始;TRUE:开始(开始后再设置为FALSE会停掉)
    objAtomic::T m_startFlag;
    DWORD m_dwTaskRefCount;
    objCounter *m_pTaskRefCountSem;
};


#endif // #ifndef _LANAPPTYPE_H_

