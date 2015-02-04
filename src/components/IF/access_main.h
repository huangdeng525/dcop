/// -------------------------------------------------
/// access_main.h : 分布式对象接入私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _ACCESS_MAIN_H_
#define _ACCESS_MAIN_H_

#include "access_if.h"
#include "ObjAttribute_if.h"
#include "session_if.h"
#include "user_if.h"
#include "sock.h"


/// 分布式对象接入管理实现类
class CAccess : public IAccess
{
public:
    /// 接入的控制/数据通道
    static const DWORD CHANNEL_CTRL = 1;
    static const DWORD CHANNEL_DATA = 2;

    /// 接入的控制/数据端口
    static const WORD PORT_CTRL = OSSOCK_DEFAULT_PORT;
    static const WORD PORT_DATA = OSSOCK_DEFAULT_PORT;

    /// 请求缓存数量
    static const DWORD REQ_POOL_COUNT = 2048;

    /// 重发次数、超时时间
    static const DWORD SEND_TRY_TIMES = 3;
    static const DWORD SEND_TIMEOUT = 5000;

    /// 网络任务数量
    static const DWORD LAN_TASK_COUNT = 2;

    /// 用户接入登录字段ID
    enum LOGIN_FIELD_ID
    {
        LOGIN_USERNAME = 1,
        LOGIN_PASSTEXT
    };

public:
    CAccess(Instance *piParent, int argc, char **argv);
    ~CAccess();

    DCOP_DECLARE_INSTANCE;
    DCOP_DECLARE_IOBJECT;

    DCOP_DECLARE_IOBJECT_MSG_HANDLE;

    DWORD Init(IObject *root, int argc, void **argv);
    void Fini();

    DWORD Input(objMsg *pMsg, DWORD dwRemoteIP, WORD wRemotePort);
    DWORD Output(objMsg *pMsg, DWORD dwRemoteIP, WORD wRemotePort);

    void OnStart(objMsg *msg);
    void OnFinish(objMsg *msg);
    void OnSessionTimeout(objMsg *msg);
    void OnResponse(objMsg *msg);
    void OnDefault(objMsg *msg);

    static void OnLogPrint(const char *cszLogInfo,
                        const char *pFile,
                        DWORD dwLine,
                        void *pUserArg);

    static DWORD OnAccept(DWORD dwChannelID,
                        objSock *pServerSock,
                        objSock *pAcceptSock,
                        void *pUserArg);

    static void OnRecv(DWORD dwChannelID,
                        objSock *pSock,
                        void *pFrameBuf,
                        DWORD dwFrameLen,
                        const char *cszRemoteIP,
                        WORD wRemotePort,
                        void *pUserArg);

    static void OnLogin(IObject *pOwner, 
                        objMsg *pMsg, 
                        const DCOP_SESSION_HEAD &sessionHead, 
                        const CDArray &aCondHeads, 
                        DCOP_PARA_NODE *pReqPara, 
                        DWORD dwReqParaCount, 
                        void *pReqData, 
                        DWORD dwReqDataLen);

    static void OnLogout(IObject *pOwner, 
                        objMsg *pMsg, 
                        const DCOP_SESSION_HEAD &sessionHead, 
                        const CDArray &aCondHeads, 
                        DCOP_PARA_NODE *pReqPara, 
                        DWORD dwReqParaCount, 
                        void *pReqData, 
                        DWORD dwReqDataLen);

    DECLARE_ATTRIBUTE_INDEX(accessIndex);
    DECLARE_ATTRIBUTE(CMethod, loginProc);
    DECLARE_ATTRIBUTE(CMethod, logoutProc);

private:
    void StartLanApp();
    void StopLanApp();

    IUser *GetUser() {return m_piUser;}
    ISession *GetSession() {return m_piSession;}
    objSock *GetDataChannel()
    {
        AutoObjLock(this);
        if (!m_pLanApp) return NULL;
        return m_pLanApp->GetChannel(CHANNEL_DATA);
    }

    DWORD CheckLogin(DWORD dwRemoteIP,
                        WORD wRemotePort,
                        ISession::NODE &rSessNode);

    DWORD DispatchMsg(objMsg *pMsg,
                        ISession::NODE *pSessNode);

private:
    DWORD m_dwLanTaskCount;                         // 网络任务数量

    IUser *m_piUser;                                // 用户管理
    ISession *m_piSession;                          // 会话管理
    IDispatch *m_piDispatch;                        // 分发器
    INotify::IPool *m_pNotifyPool;                  // 事件池

    IControl *m_piControl;                          // 控制器
    IControl::IChain *m_pCtrlChain;                 // 控制链

    IResponse *m_piResponse;                        // 响应器
    IResponse::IPool *m_pReqPool;                   // 请求缓冲池

    objLanApp *m_pLanApp;
};

#endif // #ifndef _ACCESS_MAIN_H_

