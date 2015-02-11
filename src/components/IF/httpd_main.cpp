/// -------------------------------------------------
/// httpd_main.cpp : 超文本接入实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "httpd_main.h"
#include "Factory_if.h"
#include "BaseMessage.h"
#include "ObjAttribute_if.h"


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CHttpServer, "httpd")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CHttpServer)
    DCOP_IMPLEMENT_INTERFACE(IHttpServer)
    DCOP_IMPLEMENT_INTERFACE(IObject)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 实现对象类
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT(CHttpServer)
    DCOP_IMPLEMENT_IDENTIFY_DYNAMIC("name", "id")
    DCOP_IMPLEMENT_CONFIG_THREADSAFE("threadsafe")
    DCOP_IMPLEMENT_CONFIG_CONCURRENT_ENABLE()
    DCOP_IMPLEMENT_CONFIG_INTEGER("lantaskcount", m_dwHttpdLanTaskCount)
    DCOP_IMPLEMENT_CONFIG_STRING("proctaskcount", m_szHttpdProcTaskCount)
    DCOP_IMPLEMENT_CONFIG_INTEGER("httpdport", m_wHttpdPort)
    DCOP_IMPLEMENT_CONFIG_STRING("httpddir", m_szHttpdDir)
    DCOP_IMPLEMENT_CONFIG_STRING("httpdhome", m_szHttpdHome)
DCOP_IMPLEMENT_IOBJECT_END

/// -------------------------------------------------
/// 实现消息分发
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT_MSG_HANDLE(CHttpServer)
    DCOP_IMPLEMENT_IOBJECT_MSG_PROC(DCOP_MSG_MANAGER_START, OnStart)
    DCOP_IMPLEMENT_IOBJECT_MSG_PROC(DCOP_MSG_MANAGER_FINISH, OnFinish)
    DCOP_IMPLEMENT_IOBJECT_MSG_PROC(DCOP_MSG_MODEL_REG, OnModelReg)
    DCOP_IMPLEMENT_IOBJECT_MSG_PROC(DCOP_MSG_SESSION_TIMEOUT, OnSessionTimeout)
    DCOP_IMPLEMENT_IOBJECT_MSG_PROC(DCOP_MSG_OBJECT_RESPONSE, OnResponse)
    DCOP_IMPLEMENT_IOBJECT_MSG_PROC(DCOP_MSG_OBJECT_EVENT, OnEvent)
    DCOP_IMPLEMENT_IOBJECT_MSG_DEFAULT(OnDefault)
DCOP_IMPLEMENT_IOBJECT_MSG_HANDLE_END


/*******************************************************
  函 数 名: CHttpServer::CHttpServer
  描    述: CHttpServer构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CHttpServer::CHttpServer(Instance *piParent, int argc, char **argv)
{
    m_dwHttpdLanTaskCount = LAN_TASK_COUNT;
    (void)memset(m_szHttpdProcTaskCount, 0, sizeof(m_szHttpdProcTaskCount));
    m_wHttpdPort = HTTP_PORT;
    (void)snprintf(m_szHttpdDir, sizeof(m_szHttpdDir), ".");
    (void)snprintf(m_szHttpdHome, sizeof(m_szHttpdHome), "index.html");

    m_piManager = 0;
    m_piDispatch = 0;
    m_piNotify = 0;
    m_piModel = 0;
    m_piStatus = 0;

    m_piResponse = 0;
    m_pReqPool = 0;

    m_piUser = 0;
    m_piSession = 0;
    m_piAccess = 0;

    m_pLanApp = 0;

    m_pHttpRequest = 0;
    m_pHttpProcess = 0;
    m_pHttpResponse = 0;

    m_pHttpJson = 0;

    m_pHttpSchedule = 0;

    DCOP_CONSTRUCT_INSTANCE(piParent);
    DCOP_CONSTRUCT_IOBJECT(argc, argv);
}

/*******************************************************
  函 数 名: CHttpServer::~CHttpServer
  描    述: CHttpServer析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CHttpServer::~CHttpServer()
{
    Fini();

    DCOP_DESTRUCT_IOBJECT();
    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CHttpServer::Init
  描    述: 初始化入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CHttpServer::Init(IObject *root, int argc, void **argv)
{
    if (!root)
    {
        return FAILURE;
    }

    AutoObjLock(this);

    /// 查询对象
    DCOP_QUERY_OBJECT_START(root)
        DCOP_QUERY_OBJECT_ITEM(IManager,     DCOP_OBJECT_MANAGER,    m_piManager)
        DCOP_QUERY_OBJECT_ITEM(IDispatch,    DCOP_OBJECT_DISPATCH,   m_piDispatch)
        DCOP_QUERY_OBJECT_ITEM(INotify,      DCOP_OBJECT_NOTIFY,     m_piNotify)
        DCOP_QUERY_OBJECT_ITEM(IModel,       DCOP_OBJECT_MODEL,      m_piModel)
        DCOP_QUERY_OBJECT_ITEM(IStatus,      DCOP_OBJECT_STATUS,     m_piStatus)
        DCOP_QUERY_OBJECT_ITEM(IResponse,    DCOP_OBJECT_RESPONSE,   m_piResponse)
        DCOP_QUERY_OBJECT_ITEM(IUser,        DCOP_OBJECT_USER,       m_piUser)
        DCOP_QUERY_OBJECT_ITEM(ISession,     DCOP_OBJECT_SESSION,    m_piSession)
        DCOP_QUERY_OBJECT_ITEM(IAccess,      DCOP_OBJECT_ACCESS,     m_piAccess)
    DCOP_QUERY_OBJECT_END

    /// 订阅事件
    SUBSCRIBE_EVENT_START(m_piNotify)
        SUBSCRIBE_EVENT_ITEM(DCOP_OBJECT_MODEL,     DCOP_MSG_MODEL_REG)
        SUBSCRIBE_EVENT_ITEM(DCOP_OBJECT_USER,      DCOP_OBJATTR_USER_TABLE)
        SUBSCRIBE_EVENT_ITEM(DCOP_OBJECT_SESSION,   DCOP_OBJATTR_SESSION_TABLE)
    SUBSCRIBE_EVENT_END

    /// 创建HTTP请求/处理/响应对象
    const char *httpdProcName = "restful";
    char httpdProcID[16];
    (void)snprintf(httpdProcID, sizeof(httpdProcID), "%d", DCOP_OBJECT_RESTFUL);
    char *httpdProcArg[] = 
    {
        (char *)"-name", (char *)httpdProcName, 
        (char *)"-id", httpdProcID, 
        (char *)"-dir", m_szHttpdDir, (char *)"-home", m_szHttpdHome
    };
    DCOP_CREATE_INSTANCE(IHttpRequest, "HttpRequest", this, 0, 0, m_pHttpRequest);
    DCOP_CREATE_INSTANCE(IHttpProcess, httpdProcName, this, ARRAY_SIZE(httpdProcArg), httpdProcArg, m_pHttpProcess);
    DCOP_CREATE_INSTANCE(IHttpResponse, "HttpResponse", this, 0, 0, m_pHttpResponse);
    if (!m_pHttpRequest || !m_pHttpProcess || !m_pHttpResponse)
    {
        return FAILURE;
    }

    /// 将处理对象加入到管理器中
    DWORD dwRc = m_piManager->InsertObject(m_pHttpProcess);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    /// 创建HTTP处理JSON对象(父对象是'HttpProcess')
    DCOP_CREATE_INSTANCE(IObject, "HttpJson", m_pHttpProcess, 0, 0, m_pHttpJson);
    if (!m_pHttpJson)
    {
        return FAILURE;
    }

    /// 注册资源类型和处理对象
    IHttpProcess::ResTypeNode procArg[] = 
    {
        {"json", m_pHttpJson},
    };
    dwRc = m_pHttpProcess->RegResType(procArg, ARRAY_SIZE(procArg));
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    /// 创建HTTP处理线程池调度器
    char *schdArg[] = 
    {
        (char *)"-taskname", (char *)"HttpdProc", 
        (char *)"-taskcount", m_szHttpdProcTaskCount
    };
    DCOP_CREATE_INSTANCE(ISchedule, "schedule", this, ARRAY_SIZE(schdArg), schdArg, m_pHttpSchedule);
    if (!m_pHttpSchedule)
    {
        return FAILURE;
    }

    /// 初始化HTTP处理线程池调度器
    dwRc = m_pHttpSchedule->Init(root, 0, 0);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    /// 设置HTTP处理线程池调度器
    m_pHttpSchedule->SetProc(ProcHttp, this);
    m_pHttpSchedule->SetSameDst(false);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CHttpServer::Fini
  描    述: 结束时入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpServer::Fini()
{
    OnFinish(NULL);

    if (m_pHttpProcess)
    {
        m_pHttpProcess->Fini();
    }

    DCOP_RELEASE_INSTANCE(m_pHttpJson);
    DCOP_RELEASE_INSTANCE(m_pHttpSchedule);
    DCOP_RELEASE_INSTANCE(m_pHttpRequest);
    DCOP_RELEASE_INSTANCE(m_pHttpProcess);
    DCOP_RELEASE_INSTANCE(m_pHttpResponse);

    DCOP_RELEASE_INSTANCE(m_piAccess);
    DCOP_RELEASE_INSTANCE(m_piSession);
    DCOP_RELEASE_INSTANCE(m_piUser);
    DCOP_RELEASE_INSTANCE(m_piResponse);
    DCOP_RELEASE_INSTANCE(m_piStatus);
    DCOP_RELEASE_INSTANCE(m_piModel);
    DCOP_RELEASE_INSTANCE(m_piNotify);
    DCOP_RELEASE_INSTANCE(m_piDispatch);
    DCOP_RELEASE_INSTANCE(m_piManager);
}

/*******************************************************
  函 数 名: CHttpServer::Dump
  描    述: DUMP入口
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpServer::Dump(LOG_PRINT logPrint, LOG_PARA logPara, int argc, void **argv)
{
    if (!logPrint) return;

    AutoObjLock(this);

    if (m_pHttpSchedule)
    {
        logPrint(">>>>>>>>> Http Scheduler Dump: \r\n", logPara);
        m_pHttpSchedule->Dump(logPrint, logPara, argc, argv);
    }

    logPrint(STR_FORMAT(">>>>>>>>> Http Dir: %s \r\n", m_szHttpdDir), logPara);
    logPrint(STR_FORMAT(">>>>>>>>> Http Home: %s \r\n", m_szHttpdHome), logPara);
}

/*******************************************************
  函 数 名: CHttpServer::SaveHttpToSession
  描    述: 保存HTTP句柄到会话中
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpServer::SaveHttpToSession(DWORD dwSessionID, IHttpHandle *pHttp)
{
    /// 这里是处理HTTP任务和SOCK任务都可能访问，所以要使用保护
    AutoObjLock(this);

    SessionNode *pSessNode = FindSession(dwSessionID);
    if (!pSessNode)
    {
        return;
    }

    if (pHttp == pSessNode->m_pHttp)
    {
        return;
    }

    /// 如果是设置HTTP句柄，则增加引用
    if (pHttp)
    {
        (void)pHttp->QueryInterface(ID_INTF(IHttpHandle));
        if (pSessNode->m_pHttp)
        {
            /// 这里说明是更换了HTTP句柄，需要对原HTTP句柄进行释放
            (void)pSessNode->m_pHttp->Release();
        }
    }

    /// 如果是取消HTTP句柄，则释放引用
    if (!pHttp && pSessNode->m_pHttp)
    {
        (void)pSessNode->m_pHttp->Release();
    }

    pSessNode->m_pHttp = pHttp;
}

/*******************************************************
  函 数 名: CHttpServer::OnStart
  描    述: 开始运行时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpServer::OnStart(objMsg *msg)
{
    /// 由于消息入口可以并行重入，所以访问内部对象时需要保护
    AutoObjLock(this);

    /// 创建请求缓冲池
    if (m_piResponse && !m_pReqPool)
    {
        m_pReqPool = m_piResponse->CreatePool(this, REQ_POOL_COUNT);
        if (!m_pReqPool) CHECK_ERRCODE(FAILURE, "Create Response Pool");
    }

    /// 启动网络
    StartLanApp();
}

/*******************************************************
  函 数 名: CHttpServer::OnFinish
  描    述: 结束运行时
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpServer::OnFinish(objMsg *msg)
{
    /// 由于消息入口可以并行重入，所以访问内部对象时需要保护
    AutoObjLock(this);

    /// 停止网络
    StopLanApp();

    /// 删除请求缓冲池
    if (m_piResponse && m_pReqPool)
    {
        m_piResponse->DestroyPool(m_pReqPool);
        m_pReqPool = 0;
    }
}

/*******************************************************
  函 数 名: CHttpServer::OnModelReg
  描    述: 模型注册事件
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpServer::OnModelReg(objMsg *msg)
{
    if (!msg || !m_piModel || !m_pHttpProcess)
    {
        return;
    }

    DWORD *pdwPara = (DWORD *)msg->GetDataBuf();
    if (!pdwPara) return;

    DWORD dwCount = pdwPara[0];
    if (!dwCount)
    {
        return;
    }

    IHttpProcess::ResPathNode *pNode = (IHttpProcess::ResPathNode *)DCOP_Malloc(
                        dwCount * sizeof(IHttpProcess::ResPathNode));
    if (!pNode)
    {
        return;
    }

    for (DWORD i = 0; i < dwCount; ++i)
    {
        DWORD dwAttrID = pdwPara[i + 1];

        pNode[i].m_resPath = m_piModel->GetTableName(dwAttrID);
        pNode[i].m_attrID = dwAttrID;

        RegModel(dwAttrID);
    }

    DWORD dwRc = m_pHttpProcess->RegResPath(pNode, dwCount);
    CHECK_ERRCODE(dwRc, "Reg Res Path");

    DCOP_Free(pNode);
}

/*******************************************************
  函 数 名: CHttpServer::OnSessionTimeout
  描    述: 会话超时消息
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpServer::OnSessionTimeout(objMsg *msg)
{
    if (!msg) return;

    DWORD *pdwSessID = (DWORD *)msg->GetDataBuf();
    if (!pdwSessID) return;

    /// 由于消息入口可以并行重入，所以访问内部对象时需要保护
    AutoObjLock(this);

    if (!m_piModel || !m_pHttpProcess)
    {
        return;
    }

    SessionNode *pSessNode = FindSession(*pdwSessID);
    if (!pSessNode)
    {
        return;
    }

    objSock *pSock = pSessNode->m_pSock;
    if (!pSock)
    {
        return;
    }

#if _HTTPD_DEBUG_
    PrintLog(STR_FORMAT("<Session(%d) Shutdown> %s:%d", pSessNode->m_dwSessID, 
                        pSock->cszGetHostIP(), pSock->wGetHostPort()), 
                        PrintToConsole, 0);
#endif

    /// 由发送线程进行优雅关闭，稍后会由接收线程自己关闭
    (void)pSock->Shut();
}

/*******************************************************
  函 数 名: CHttpServer::OnResponse
  描    述: 响应消息处理
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpServer::OnResponse(objMsg *msg)
{
    if (!msg) return;

    /// 解析会话头
    CDArray aSessHeads;
    IObjectMember::GetMsgHead(msg->GetDataBuf(), msg->GetDataLen(), &aSessHeads, 0, 0, 0, 0);
    DCOP_SESSION_HEAD *pSessionHead = (DCOP_SESSION_HEAD *)aSessHeads.Pos(0);
    if (!pSessionHead)
    {
        return;
    }

    /// 由于消息入口可以并行重入，所以访问内部对象时需要保护
    AutoObjLock(this);

#if _HTTPD_DEBUG_ && _HTTPD_DEBUG_DETAIL_
    PrintBuffer(STR_FORMAT("<Recv Ack Msg> len:%d type:0x%x src:%d, dst:%d", 
                        msg->GetDataLen(), msg->GetMsgType(), msg->GetSrcID(), msg->GetDstID()), 
                        msg->GetDataBuf(), msg->GetDataLen(), 
                        PrintToConsole, 0);
#endif

    /// 查找本地会话
    SessionNode *pSessNode = FindSession(pSessionHead->m_session);
    if (!pSessNode)
    {
        return;
    }

    /// 继续数据处理
    ProcData(*pSessNode, DCOP_MSG_HTTPD_PROCESS, msg->GetDataBuf(), msg->GetDataLen());
}

/*******************************************************
  函 数 名: CHttpServer::OnEvent
  描    述: 事件消息处理
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpServer::OnEvent(objMsg *msg)
{
}

/*******************************************************
  函 数 名: CHttpServer::OnHttpRequest
  描    述: Http请求消息
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpServer::OnHttpRequest(objMsg *msg)
{
    if (!msg || !m_pHttpRequest)
    {
        return;
    }

    /// 消息的控制参数区保存的是会话信息
    DWORD dwCtrlLen = 0;
    SessionNode *pSessNode = (SessionNode *)msg->GetCtrl(dwCtrlLen);
    if (!pSessNode || (dwCtrlLen < sizeof(SessionNode)))
    {
        return;
    }

    /// 无网络连接就不能处理请求了
    objSock *pSock = pSessNode->m_pSock;
    if (!pSock)
    {
        return;
    }

    /// 将输入消息转换为字符串，并进行解码
    CDString strReq((const char *)msg->GetDataBuf(), msg->GetDataLen());
    if (!strReq.Length())
    {
        return;
    }

    CHttpRequest::StrDecode(strReq.Get());

#if _HTTPD_DEBUG_
    PrintLog(STR_FORMAT("<Session(%d) Recv Http Req> Len:%d From:'%s:%d'", 
                        pSessNode->m_dwSessID, msg->GetDataLen(), pSock->cszGetHostIP(), pSock->wGetHostPort()), 
                        PrintToConsole, 0);
    if ((const char *)strReq) PrintToConsole((const char *)strReq, 0);
#endif

    /// 获取会话信息中的HTTP句柄，没有就新创建一个
    IHttpHandle *pHttp = pSessNode->m_pHttp;
    if (!pHttp)
    {
        pHttp = CREATE_HTTP_HANDLE();
        if (!pHttp) return;
        pSessNode->m_pHttp = pHttp;
    }

    /// 进行输入处理
    m_pHttpRequest->Input(pHttp, (const char *)strReq);

    /// 如果不完整，则需要返回等待下一次输入
    if (!pHttp->bCompleted())
    {
        /// 由于下次要使用，只能先保存回原始会话中(因为请求阶段的句柄在取出时都会被清空)
        SaveHttpToSession(pSessNode->m_dwSessID, pHttp);
        return;
    }

    /// 设置会话信息
    if (!pSessNode->m_bSetClientInfo)
    {
        SetSessionInfo(pSessNode->m_dwSessID, pHttp->GetUserAgent());
    }

    /// 接着进行处理
    if (pHttp->GetStatus() == IHttpHandle::STATUS_PROCESS)
    {
        msg->MsgHead().m_dwMsgType = DCOP_MSG_HTTPD_PROCESS;
    }
    else
    {
        msg->MsgHead().m_dwMsgType = DCOP_MSG_HTTPD_RESPONSE;
    }
}

/*******************************************************
  函 数 名: CHttpServer::OnHttpProcess
  描    述: Http处理消息
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpServer::OnHttpProcess(objMsg *msg)
{
    if (!msg || !m_pHttpProcess)
    {
        return;
    }

    /// 消息的控制参数区保存的是会话信息
    DWORD dwCtrlLen = 0;
    SessionNode *pSessNode = (SessionNode *)msg->GetCtrl(dwCtrlLen);
    if (!pSessNode || (dwCtrlLen < sizeof(SessionNode)))
    {
        return;
    }

    /// 获取会话信息中的HTTP句柄
    IHttpHandle *pHttp = pSessNode->m_pHttp;
    if (!pHttp)
    {
        return;
    }

    /// 进行内容处理
    m_pHttpProcess->Proc(msg);

    /// 接着进行响应
    if (pHttp->GetStatus() >= IHttpHandle::STATUS_RESPONSE)
    {
        msg->MsgHead().m_dwMsgType = DCOP_MSG_HTTPD_RESPONSE;
    }
}

/*******************************************************
  函 数 名: CHttpServer::OnHttpResponse
  描    述: Http响应消息
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpServer::OnHttpResponse(objMsg *msg)
{
    if (!msg || !m_pHttpResponse)
    {
        return;
    }

    /// 消息的控制参数区保存的是会话信息
    DWORD dwCtrlLen = 0;
    SessionNode *pSessNode = (SessionNode *)msg->GetCtrl(dwCtrlLen);
    if (!pSessNode || (dwCtrlLen < sizeof(SessionNode)))
    {
        return;
    }

    /// 无网络连接就不能处理响应了
    objSock *pSock = pSessNode->m_pSock;
    if (!pSock)
    {
        return;
    }

    /// 获取会话信息中的HTTP句柄
    IHttpHandle *pHttp = pSessNode->m_pHttp;
    if (!pHttp)
    {
        return;
    }

    /// 进行响应输出
    CDStream sRsp;
    DWORD dwHeadSize = 0;
    m_pHttpResponse->Output(pHttp, sRsp, &dwHeadSize);

    /// Send里面会进行重试，这里只需要设置每次等待时间即可
    DWORD dwSent = 0;
    DWORD dwRc = pSock->Send(sRsp.Buffer(), sRsp.Length(), &dwSent, true, RSP_WAIT_TIME);
#if _HTTPD_DEBUG_
    PrintLog(STR_FORMAT("<Session(%d) Send Http Rsp> Len:%d Sent:%d bCompleted:%d[%d/%d] Rc:%d To:'%s:%d'", 
                        pSessNode->m_dwSessID, sRsp.Length(), dwSent, pHttp->bCompleted(), 
                        pHttp->Content().Length(), pHttp->GetContentLength(), dwRc, 
                        pSock->cszGetHostIP(), pSock->wGetHostPort()), 
                        PrintToConsole, 0);
    CDString strRsp((const char *)sRsp.Buffer(), dwHeadSize);
    if ((const char *)strRsp) PrintToConsole((const char *)strRsp, 0);
#endif

    /// 如果发送失败，或者已经发送完毕，退出
    if ((dwRc != SUCCESS) || (pHttp->bCompleted()))
    {
        return;
    }

    /// 这里接着处理未发送完毕的响应内容
    DWORD dwOffset = pHttp->Content().Length();
    DWORD dwLength = pHttp->GetContentLength();
    if (dwOffset >= dwLength) return;
    while (dwOffset < dwLength)
    {
        sRsp.Clear();
        if (sRsp.LoadFile(pHttp->GetURI(), dwOffset, DSTREAM_DMEM_MAX_LEN) != SUCCESS)
        {
            break;
        }

        /// Send里面会进行重试，这里只需要设置每次等待时间即可
        dwSent = 0;
        dwRc = pSock->Send(sRsp.Buffer(), sRsp.Length(), &dwSent, true, RSP_WAIT_TIME);
    #if _HTTPD_DEBUG_
        PrintLog(STR_FORMAT("<Session(%d) Send Http Rsp Continue> Offset:%d Len:%d Sent:%d Rc:%d To:'%s:%d'", 
                        pSessNode->m_dwSessID, dwOffset, sRsp.Length(), dwSent, dwRc, 
                        pSock->cszGetHostIP(), pSock->wGetHostPort()), 
                        PrintToConsole, 0);
    #endif
        if (dwRc != SUCCESS)
        {
            /// Send里面会重试很多次，这里失败只有退出了
            break;
        }

        dwOffset += sRsp.Length();
    }
}

/*******************************************************
  函 数 名: CHttpServer::OnDefault
  描    述: 默认消息处理
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpServer::OnDefault(objMsg *msg)
{
}

/*******************************************************
  函 数 名: CHttpServer::RegModel
  描    述: 注册模型
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpServer::RegModel(DWORD dwAttrID)
{
    if (!m_piModel) return;

    /// 由于消息入口可以并行重入，所以访问内部对象时需要保护
    AutoObjLock(this);

    std::string strKey = m_piModel->GetTableName(dwAttrID);
    (void)m_objattrs.insert(MAP_OBJATTRS::value_type(strKey, dwAttrID));
}

/*******************************************************
  函 数 名: CHttpServer::StartLanApp
  描    述: 启动网络应用
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpServer::StartLanApp()
{
    TRACE_LOG("HttpServer Network Service Start ...");

    m_pLanApp = objLanApp::CreateInstance();
    if (!m_pLanApp)
    {
        TRACE_LOG("HttpServer Service Init Fail!");
    }

    DWORD dwRc = m_pLanApp->AddTCPServer(HTTP_CHANNEL, m_wHttpdPort);
    TRACE_LOG(STR_FORMAT("HttpServer Service Add TCPServer(channel:%d,port:%d) rc:0x%x!", HTTP_CHANNEL, m_wHttpdPort, dwRc));

    objLanApp::LanEventProc theEventProc = 
    {
        OnAccept, this,
        NULL, 0,
        OnDisconnect, this,
        OnRecv, this
    };

    objLanApp::LanFrameProc theFrameProc = 
    {
        bFrame, this,
        BytesOrder, this
    };

    objLanApp::LanLogProc theLogProc = 
    {
        OnLogPrint, this
    };

    IObject *piParent = Parent();
    dwRc = m_pLanApp->Start("HttpdLan",
                        (piParent)? piParent->ID() : 0,
                        ID(),
                        m_dwHttpdLanTaskCount,
                        &theEventProc,
                        &theFrameProc,
                        &theLogProc);
    TRACE_LOG(STR_FORMAT("HttpServer Service Start Rc:0x%x!", dwRc));
}

/*******************************************************
  函 数 名: CHttpServer::StopLanApp
  描    述: 停止网络应用
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpServer::StopLanApp()
{
    if (!m_pLanApp)
    {
        return;
    }

    m_pLanApp->Stop();
    delete m_pLanApp;
    m_pLanApp = 0;

    TRACE_LOG("HttpServer Network Service Stop!");
}

/*******************************************************
  函 数 名: CHttpServer::OnLogPrint
  描    述: 日志打印回调
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpServer::OnLogPrint(const char *cszLogInfo,
                        const char *pFile,
                        DWORD dwLine,
                        void *pUserArg)
{
    TraceLogEx(cszLogInfo, pFile, dwLine);
}

/*******************************************************
  函 数 名: CHttpServer::OnAccept
  描    述: 接收连接
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CHttpServer::OnAccept(DWORD dwChannelID,
                        objSock *pServerSock,
                        objSock *pAcceptSock,
                        void *pUserArg)
{
    CHttpServer *pThis = (CHttpServer *)pUserArg;
    if (!pThis)
    {
        return FAILURE;
    }

    AutoObjLock(pThis);

    DWORD dwRc = pThis->CreateSession(pAcceptSock);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CHttpServer::OnDisconnect
  描    述: 断开连接
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpServer::OnDisconnect(DWORD dwChannelID,
                        objSock *pSock,
                        const char *cszRemoteIP,
                        WORD wRemotePort,
                        void *pUserArg)
{
    CHttpServer *pThis = (CHttpServer *)pUserArg;
    if (!pThis)
    {
        return;
    }

    AutoObjLock(pThis);

    pThis->DeleteSession(objSock::GetIPValueByString(cszRemoteIP), wRemotePort);
}

/*******************************************************
  函 数 名: CHttpServer::OnRecv
  描    述: 接收数据
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpServer::OnRecv(DWORD dwChannelID,
                        objSock *pSock,
                        void *pFrameBuf,
                        DWORD dwFrameLen,
                        const char *cszRemoteIP,
                        WORD wRemotePort,
                        void *pUserArg)
{
    CHttpServer *pThis = (CHttpServer *)pUserArg;
    if (!pThis)
    {
        return;
    }

    AutoObjLock(pThis);

    SessionNode *pSessNode = pThis->FindSession(
                        objSock::GetIPValueByString(cszRemoteIP), 
                        wRemotePort);
#if _HTTPD_DEBUG_ && _HTTPD_DEBUG_DETAIL_
    PrintBuffer(STR_FORMAT("=== %s:%d -> Session(%d) \r\n", cszRemoteIP, wRemotePort, 
                        (pSessNode)? pSessNode->m_dwSessID : (-1)), pFrameBuf, dwFrameLen, 
                        PrintToConsole, 0);
#endif
    if (!pSessNode)
    {
        return;
    }

    pThis->ProcData(*pSessNode, DCOP_MSG_HTTPD_REQUEST, pFrameBuf, dwFrameLen);
}

/*******************************************************
  函 数 名: CHttpServer::bFrame
  描    述: 判断数据帧
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
int CHttpServer::bFrame(void *pBuf,
                        DWORD dwLen,
                        void *pUserArg)
{
    return dwLen;
}

/*******************************************************
  函 数 名: CHttpServer::BytesOrder
  描    述: 接收数据
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpServer::BytesOrder(void *pBuf,
                    DWORD dwLen,
                    void *pUserArg)
{
}

/*******************************************************
  函 数 名: CHttpServer::CreateSession
  描    述: 创建会话
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CHttpServer::CreateSession(objSock *pSock)
{
    if (!pSock || !m_piSession) return FAILURE;

    DWORD dwSessionID = 0;
    ISession::NODE sessNode;
    (void)memset(&sessNode, 0, sizeof(sessNode));

    /// 查找会话是否存在
    DWORD dwIP = objSock::GetIPValueByString(pSock->cszGetHostIP());
    WORD wPort = pSock->wGetHostPort();
    DWORD dwRc = m_piSession->FindSession(dwIP, wPort, sessNode);
    if (dwRc == SUCCESS)
    {
        /// 已经添加到会话中了
    #if _HTTPD_DEBUG_
        PrintLog(STR_FORMAT("<Session(%d) Exist> %s:%d", sessNode.SessID, 
                        pSock->cszGetHostIP(), pSock->wGetHostPort()), 
                        PrintToConsole, 0);
    #endif
        return SUCCESS;
    }

    /// 不存在的会话，添加为未登录用户
    dwRc = m_piSession->CreateSession(DCOP_USER_UNLOGIN, 
                        DCOP_GROUP_VISITOR, 
                        ID(), 
                        dwIP, 
                        wPort, 
                        dwSessionID);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    SessionNode sess;
    (void)memset(sess.m_szUserName, 0, sizeof(sess.m_szUserName));
    sess.m_dwUserID = DCOP_USER_UNLOGIN;
    sess.m_dwUserGroup = DCOP_GROUP_VISITOR;
    sess.m_dwSessID = dwSessionID;
    sess.m_pSock = pSock;
    sess.m_pHttp = 0;
    sess.m_bSetClientInfo = false;
    (void)m_sessions.insert(MAP_SESSIONS::value_type(dwSessionID, sess));

#if _HTTPD_DEBUG_
    PrintLog(STR_FORMAT("<Session(%d) Connected> %s:%d", dwSessionID, 
                        pSock->cszGetHostIP(), pSock->wGetHostPort()), 
                        PrintToConsole, 0);
#endif

    return SUCCESS;
}

/*******************************************************
  函 数 名: CHttpServer::DeleteSession
  描    述: 删除会话
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpServer::DeleteSession(DWORD dwIP, WORD wPort)
{
    if (!m_piSession) return;

    /// 查找会话是否存在
    ISession::NODE sessNode;
    DWORD dwRc = m_piSession->FindSession(dwIP, wPort, sessNode);
    if (dwRc != SUCCESS)
    {
        return;
    }

    (void)m_piSession->DeleteSession(sessNode.SessID);

    IT_SESSIONS it_sess = m_sessions.find(sessNode.SessID);
    if (it_sess == m_sessions.end())
    {
        return;
    }

    IHttpHandle *pHttp = ((*it_sess).second).m_pHttp;
    if (pHttp)
    {
        (void)pHttp->Release();
    }

    (void)m_sessions.erase(sessNode.SessID);

#if _HTTPD_DEBUG_
    char szIP[OSSOCK_IPSIZE] = {0,};
    objSock::GetIPStringByValue(dwIP, szIP);
    PrintLog(STR_FORMAT("<Session(%d) Disconnected> %s:%d", sessNode.SessID, 
                        szIP, wPort), 
                        PrintToConsole, 0);
#endif
}

/*******************************************************
  函 数 名: CHttpServer::FindSession
  描    述: 查找会话
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CHttpServer::SessionNode *CHttpServer::FindSession(DWORD dwIP, WORD wPort)
{
    ISession::NODE sessNode;

    /// 查找会话是否存在
    DWORD dwRc = m_piSession->FindSession(dwIP, wPort, sessNode);
    if (dwRc != SUCCESS)
    {
        return NULL;
    }

    IT_SESSIONS it_sess = m_sessions.find(sessNode.SessID);
    if (it_sess == m_sessions.end())
    {
        return NULL;
    }

    return &((*it_sess).second);
}

/*******************************************************
  函 数 名: CHttpServer::FindSession
  描    述: 查找会话
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CHttpServer::SessionNode *CHttpServer::FindSession(DWORD dwSessionID)
{
    IT_SESSIONS it_sess = m_sessions.find(dwSessionID);
    if (it_sess == m_sessions.end())
    {
        return NULL;
    }

    return &((*it_sess).second);
}

/*******************************************************
  函 数 名: CHttpServer::SetSessionInfo
  描    述: 设置会话信息
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpServer::SetSessionInfo(DWORD dwSessionID, const char *cszUserAgent)
{
    if (!cszUserAgent) return;

    /// UA信息很乱，和历史相互兼容原因有关，稍微有用的系统信息在第一个括号内
    const char *pcszRealInfo = strchr(cszUserAgent, '(');
    if (!pcszRealInfo) pcszRealInfo = cszUserAgent;
    else pcszRealInfo++;

    AutoObjLock(this);

    if (!m_piSession)
    {
        return;
    }

    SessionNode *pSessNode = FindSession(dwSessionID);
    if (!pSessNode)
    {
        return;
    }

    char szInfo[ISession::INFO_SIZE];
    (void)snprintf(szInfo, sizeof(szInfo), "%s", pcszRealInfo);

    pSessNode->m_bSetClientInfo = (m_piSession->SetSessionInfo(dwSessionID, 
                        szInfo) == SUCCESS)? true : false;
}

/*******************************************************
  函 数 名: CHttpServer::ProcData
  描    述: 处理数据
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpServer::ProcData(SessionNode &sessNode, DWORD dwMsgType, void *pFrameBuf, DWORD dwFrameLen)
{
    if (!sessNode.m_pSock || !pFrameBuf || !dwFrameLen) return;

    if (m_piSession)
    {
        m_piSession->UpdateSession(sessNode.m_dwSessID);
    }

    if (!m_pHttpSchedule)
    {
        return;
    }

    objMsg *pMsg = DCOP_LoadMsg(dwFrameLen, dwMsgType, ID(), pFrameBuf, dwFrameLen);
    if (!pMsg)
    {
        return;
    }

    /// 把会话信息设置到消息控制区
    SessionNode *pSessNode = (SessionNode *)pMsg->SetCtrl(&sessNode, sizeof(SessionNode));
    if (!pSessNode)
    {
        delete pMsg;
        return;
    }

    /// 复制会话信息中的SOCK，以便可以在处理任务中直接回复响应
    objSock *pCopySock = DCOP_CreateSock(objSock::OSSOCK_NONE, 
                        sessNode.m_pSock->cszGetHostIP(), 
                        sessNode.m_pSock->wGetHostPort());
    if (!pCopySock)
    {
        delete pMsg;
        return;
    }

    /// 对复制后的会话中的HTTP句柄进行引用
    if (pSessNode->m_pHttp)
    {
        (void)pSessNode->m_pHttp->QueryInterface(ID_INTF(IHttpHandle));

        /// 去掉请求状态的会话中的HTTP句柄
        if (dwMsgType == DCOP_MSG_HTTPD_REQUEST)
        {
            SaveHttpToSession(pSessNode->m_dwSessID, NULL);
        }
    }

    /// 最关键的是把SOCK句柄复制下来，方便多线程发送使用
    pCopySock->vSetSock(sessNode.m_pSock->sGetSock());
    pSessNode->m_pSock = pCopySock;

    pMsg->MsgHead().m_dwDstID = ID();
    if (m_pHttpSchedule->Join(pMsg) != SUCCESS)
    {
        delete pCopySock;
        delete pMsg;
    }
}

/*******************************************************
  函 数 名: CHttpServer::ProcHttp
  描    述: 处理HTTP报文
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpServer::ProcHttp(objMsg *pMsg, void *pUserArg)
{
    if (!pMsg || !pUserArg) return;

    CHttpServer *pThis = (CHttpServer *)pUserArg;
    if (!pThis)
    {
        return;
    }

#if _HTTPD_DEBUG_ && _HTTPD_DEBUG_DETAIL_
    DWORD dwCtrlLenTmp = 0;
    SessionNode *pSessNodeTmp = (SessionNode *)pMsg->GetCtrl(dwCtrlLenTmp);
    PrintBuffer(STR_FORMAT("... ProcHttp Session(%d) \r\n", pSessNodeTmp->m_dwSessID), 
                        pMsg->GetDataBuf(), pMsg->GetDataLen(), 
                        PrintToConsole, 0);
#endif

    /// 依次进行如下处理

    if (pMsg->GetMsgType() == DCOP_MSG_HTTPD_REQUEST)
    {
        pThis->OnHttpRequest(pMsg);
    }

    if (pMsg->GetMsgType() == DCOP_MSG_HTTPD_PROCESS)
    {
        pThis->OnHttpProcess(pMsg);
    }

    if (pMsg->GetMsgType() == DCOP_MSG_HTTPD_RESPONSE)
    {
        pThis->OnHttpResponse(pMsg);
    }

    /// 消息控制区的SOCK是另外复制的，之后不用了，需要主动释放
    DWORD dwCtrlLen = 0;
    SessionNode *pSessNode = (SessionNode *)pMsg->GetCtrl(dwCtrlLen);
    if (!pSessNode || (dwCtrlLen < sizeof(SessionNode)))
    {
        return;
    }

    /// HTTP句柄是引用的，释放引用
    if (pSessNode->m_pHttp)
    {
        DWORD dwRefCount = pSessNode->m_pHttp->Release();

        /// 如果句柄不为空，则同时去掉响应状态的会话中的HTTP句柄
        if (dwRefCount && (pMsg->GetMsgType() == DCOP_MSG_HTTPD_RESPONSE))
        {
            pThis->SaveHttpToSession(pSessNode->m_dwSessID, NULL);
        }
    }

    /// SOCK句柄是复制的，释放空间
    if (pSessNode->m_pSock)
    {
        delete pSessNode->m_pSock;
        pSessNode->m_pSock = 0;
    }
}

