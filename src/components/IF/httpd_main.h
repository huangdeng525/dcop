/// -------------------------------------------------
/// httpd_main.h : 超文本接入私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _HTTPD_MAIN_H_
#define _HTTPD_MAIN_H_

#define INC_STRING
#define INC_MAP

#include "httpd_if.h"
#include "httpd_protocol.h"
#include "httpd_restful.h"
#include "Manager_if.h"
#include "ObjDispatch_if.h"
#include "ObjNotify_if.h"
#include "ObjModel_if.h"
#include "ObjStatus_if.h"
#include "ObjResponse_if.h"
#include "ObjSchedule_if.h"
#include "session_if.h"
#include "access_if.h"


/// -------------------------------------------------
/// 超文本接入管理实现类
/// -------------------------------------------------
class CHttpServer : public IHttpServer
{
public:
    /// 请求缓存数量
    static const DWORD REQ_POOL_COUNT = 2048;

    /// Http Server通道和端口
    static const DWORD HTTP_CHANNEL = 1;
    static const WORD HTTP_PORT = 80;

    /// 网络任务数量
    static const DWORD LAN_TASK_COUNT = 8;

    /// 响应发送等待时间
    static const DWORD RSP_WAIT_TIME = 500;

    /// 属性列表MAP
    typedef std::map<std::string, DWORD> MAP_OBJATTRS;
    typedef MAP_OBJATTRS::iterator IT_OBJATTRS;

    /// 会话列表
    typedef std::map<DWORD, SessionNode> MAP_SESSIONS;
    typedef MAP_SESSIONS::iterator IT_SESSIONS;

public:
    CHttpServer(Instance *piParent, int argc, char **argv);
    ~CHttpServer();

    DCOP_DECLARE_INSTANCE;
    DCOP_DECLARE_IOBJECT;

    DCOP_DECLARE_IOBJECT_MSG_HANDLE;

    DWORD Init(IObject *root, int argc, void **argv);
    void Fini();
    void Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv);

    void SaveHttpToSession(DWORD dwSessionID, IHttpHandle *pHttp);

    void OnStart(objMsg *msg);
    void OnFinish(objMsg *msg);
    void OnModelReg(objMsg *msg);
    void OnSessionTimeout(objMsg *msg);
    void OnResponse(objMsg *msg);
    void OnEvent(objMsg *msg);
    void OnHttpRequest(objMsg *msg);
    void OnHttpProcess(objMsg *msg);
    void OnHttpResponse(objMsg *msg);
    void OnDefault(objMsg *msg);

    static void OnLogPrint(const char *cszLogInfo,
                        const char *pFile,
                        DWORD dwLine,
                        void *pUserArg);

    static DWORD OnAccept(DWORD dwChannelID,
                        objSock *pServerSock,
                        objSock *pAcceptSock,
                        void *pUserArg);

    static void OnDisconnect(DWORD dwChannelID,
                        objSock *pSock,
                        const char *cszRemoteIP,
                        WORD wRemotePort,
                        void *pUserArg);

    static void OnRecv(DWORD dwChannelID,
                        objSock *pSock,
                        void *pFrameBuf,
                        DWORD dwFrameLen,
                        const char *cszRemoteIP,
                        WORD wRemotePort,
                        void *pUserArg);

    static int bFrame(void *pBuf,
                        DWORD dwLen,
                        void *pUserArg);

    static void BytesOrder(void *pBuf,
                        DWORD dwLen,
                        void *pUserArg);

    static void ProcHttp(objMsg *pMsg,
                        void *pUserArg);

private:
    void RegModel(DWORD dwAttrID);

    void StartLanApp();
    void StopLanApp();

    DWORD CreateSession(objSock *pSock);
    void DeleteSession(DWORD dwIP, WORD wPort);
    SessionNode *FindSession(DWORD dwIP, WORD wPort);
    SessionNode *FindSession(DWORD dwSessionID);
    void SetSessionInfo(DWORD dwSessionID, const char *cszUserAgent);

    void ProcData(SessionNode &sessNode, DWORD dwMsgType, void *pFrameBuf, DWORD dwFrameLen);

private:
    MAP_OBJATTRS m_objattrs;                        // 属性列表

    DWORD m_dwHttpdLanTaskCount;                    // HTTP网络任务数
    char m_szHttpdProcTaskCount[6];                 // HTTP处理任务数
    WORD m_wHttpdPort;                              // HTTP服务端口
    char m_szHttpdDir[DCOP_FILE_NAME_LEN];          // HTTP目录
    char m_szHttpdHome[DCOP_FILE_NAME_LEN];         // HTTP主页

    IManager *m_piManager;                          // 管理器
    IDispatch *m_piDispatch;                        // 分发器
    INotify *m_piNotify;                            // 订阅器
    IModel *m_piModel;                              // 模型
    IStatus *m_piStatus;                            // 状态

    IResponse *m_piResponse;                        // 响应器
    IResponse::IPool *m_pReqPool;                   // 请求缓冲池

    IUser *m_piUser;                                // 用户管理
    ISession *m_piSession;                          // 会话管理
    IAccess *m_piAccess;                            // 接入管理

    objLanApp *m_pLanApp;                           // 网络应用
    MAP_SESSIONS m_sessions;                        // 本地会话信息

    IHttpRequest *m_pHttpRequest;                   // HTTP请求对象
    IHttpProcess *m_pHttpProcess;                   // HTTP处理对象
    IHttpResponse *m_pHttpResponse;                 // HTTP响应对象

    IObject *m_pHttpJson;                           // HTTP处理Json对象

    ISchedule *m_pHttpSchedule;                     // HTTP处理线程池调度器
};


#endif // #ifndef _HTTPD_MAIN_H_

