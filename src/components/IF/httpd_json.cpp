/// -------------------------------------------------
/// httpd_json.cpp : 超文本处理JSON实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include <json/json.h>
#include "httpd_json.h"
#include "Factory_if.h"
#include "command_main.h"
#include "ObjAttribute_if.h"

#if DCOP_OS == DCOP_OS_WINDOWS
#pragma comment(lib, "jsond")
#endif


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CHttpJson, "HttpJson")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CHttpJson)
    DCOP_IMPLEMENT_INTERFACE(IObject)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 实现对象类
/// -------------------------------------------------
DCOP_IMPLEMENT_IOBJECT(CHttpJson)
    DCOP_IMPLEMENT_IDENTIFY_DYNAMIC("name", "id")
    DCOP_IMPLEMENT_CONFIG_THREADSAFE_ENABLE()
    DCOP_IMPLEMENT_CONFIG_CONCURRENT_ENABLE()
DCOP_IMPLEMENT_IOBJECT_END


/*******************************************************
  函 数 名: CHttpJson::CHttpJson
  描    述: CHttpJson构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CHttpJson::CHttpJson(Instance *piParent, int argc, char **argv)
{
    DCOP_CONSTRUCT_INSTANCE(piParent);
    DCOP_CONSTRUCT_IOBJECT(argc, argv);
}

/*******************************************************
  函 数 名: CHttpJson::~CHttpJson
  描    述: CHttpJson析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CHttpJson::~CHttpJson()
{
    DCOP_DESTRUCT_IOBJECT();
    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CHttpJson::Proc
  描    述: 处理JSON资源
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpJson::Proc(objMsg *msg)
{
    /////////////////////////////////////////////////
    /// 需要支持消息并发，消息入口不能使用大锁进行保护
    /////////////////////////////////////////////////

    /// 消息的控制参数区保存的是会话信息
    DWORD dwCtrlLen = 0;
    IHttpServer::SessionNode *pSessNode = (IHttpServer::SessionNode *)msg->GetCtrl(dwCtrlLen);
    if (!pSessNode || (dwCtrlLen < sizeof(IHttpServer::SessionNode)))
    {
        return;
    }

    /// 获取会话信息中的HTTP句柄
    IHttpHandle *http = pSessNode->m_pHttp;
    if (!http)
    {
        return;
    }

#if _HTTPD_DEBUG_
    PrintLog(STR_FORMAT("<Session(%d) Json Proc Start> Status:%d", 
                pSessNode->m_dwSessID, http->GetStatus()), 
                PrintToConsole, 0);
#endif

    switch (http->GetStatus())
    {
        case IHttpHandle::STATUS_PROCESS:
            ProcReq(msg);
            break;
        case IHttpHandle::STATUS_JSON:
            ProcRsp(msg);
            break;
        default:
            break;
    }

#if _HTTPD_DEBUG_
    PrintLog(STR_FORMAT("<Session(%d) Json Proc End> Status:%d", 
                pSessNode->m_dwSessID, http->GetStatus()), 
                PrintToConsole, 0);
#endif
}

/*******************************************************
  函 数 名: CHttpJson::ProcReq
  描    述: 处理JSON请求
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpJson::ProcReq(objMsg *msg)
{
    if (!msg) return;

    CHttpProcess *pHttpProcess = (CHttpProcess *)Parent();
    if (!pHttpProcess)
    {
        return;
    }

    IHttpServer *pHttpServer = (IHttpServer *)pHttpProcess->Parent();
    if (!pHttpServer)
    {
        return;
    }

    /// 消息的控制参数区保存的是会话信息
    DWORD dwCtrlLen = 0;
    IHttpServer::SessionNode *pSessNode = (IHttpServer::SessionNode *)msg->GetCtrl(dwCtrlLen);
    if (!pSessNode || (dwCtrlLen < sizeof(IHttpServer::SessionNode)))
    {
        return;
    }

    /// 获取会话信息中的HTTP句柄
    IHttpHandle *http = pSessNode->m_pHttp;
    if (!http)
    {
        return;
    }

    /// sock句柄不能为空
    if (!pSessNode->m_pSock)
    {
        return;
    }

    http->Content().Clear();
    http->EnableDynamic();
    http->SetStatus(IHttpHandle::STATUS_JSON);
    pHttpServer->SaveHttpToSession(pSessNode->m_dwSessID, http);

    /// 获取文档方法
    BYTE byOpType = DCOP_CTRL_GET;
    if (http->GetMethod() == IHttpHandle::METHOD_GET)
    {
        byOpType = DCOP_CTRL_GET;
    }
    else if (http->GetMethod() == IHttpHandle::METHOD_POST)
    {
        byOpType = DCOP_CTRL_SET;
    }
    else if (http->GetMethod() == IHttpHandle::METHOD_PUT)
    {
        byOpType = DCOP_CTRL_ADD;
    }
    else if (http->GetMethod() == IHttpHandle::METHOD_DELETE)
    {
        byOpType = DCOP_CTRL_DEL;
    }
    else
    {
        return;
    }

    /// 获取资源所有者
    DWORD dwAttrID = 0;
    IObject *owner = pHttpProcess->GetResOwner(http->GetURI(), dwAttrID);
    if (!owner)
    {
        return;
    }

    IModel *piModel = pHttpProcess->GetModel();
    if (!piModel)
    {
        return;
    }

    /// 获取字段集
    DWORD dwFieldCount = 0;
    IModel::Field *pField = piModel->GetFields(dwAttrID, dwFieldCount);
    if (!pField || !dwFieldCount)
    {
        return;
    }

    /// 获取请求参数
    DWORD dwReqParaCount = 0;
    CDStream sReqParaData;
    bool bOneValue = (http->GetMethod() != IHttpHandle::METHOD_GET)? true : false;
    DCOP_PARA_NODE *pReqParaNode = CCommand::GetArgList(http->GetReqParamList(), 
                        pField, dwFieldCount, 
                        dwReqParaCount, sReqParaData, bOneValue);

    /// 获取条件参数
    DWORD dwCondParaCount = 0;
    CDStream sCondParaData;
    DCOP_PARA_NODE *pCondParaNode = CCommand::GetArgList(http->GetCondParamList(), 
                        pField, dwFieldCount, 
                        dwCondParaCount, sCondParaData);

    /// 组装会话消息
    CAttribute::PACK_SESS_HEAD sessHead;
    sessHead.m_group = (BYTE)pSessNode->m_dwUserGroup;
    sessHead.m_session = pSessNode->m_dwSessID;
    sessHead.m_user = pSessNode->m_dwUserID;
    sessHead.m_tty = DCOP_OBJECT_HTTPD;
    sessHead.m_attribute = dwAttrID;
    sessHead.m_ctrl = byOpType;

    CAttribute::PACK_REQ_HEAD reqHead;
    reqHead.m_paraCount = (WORD)dwReqParaCount;
    reqHead.m_paraLen = (WORD)sReqParaData.Length();

    CAttribute::PACK_COND_HEAD condHead;
    condHead.m_condition = http->GetCondition();
    condHead.m_paraCount = (BYTE)dwCondParaCount;
    condHead.m_paraLen = (WORD)sCondParaData.Length();

    CAttribute::PACK_MSG_NODE packNodes[] = 
    {
        {(DCOP_MSG_HEAD *)&condHead, pCondParaNode, dwCondParaCount, sCondParaData},
        {(DCOP_MSG_HEAD *)&reqHead, pReqParaNode, dwReqParaCount, sReqParaData},
    };

    /// 打包并向内部转发
    DWORD dwRc = SUCCESS;
    do
    {
        objMsg *pMsg = NULL;
        dwRc = IObjectMember::PackMsg(NULL, &pMsg, DCOP_OBJECT_HTTPD, owner->ID(), DCOP_MSG_OBJECT_REQUEST, 
                        &sessHead, packNodes, ARRAY_SIZE(packNodes));
        if (dwRc != SUCCESS)
        {
            break;
        }

        if (!pMsg)
        {
            dwRc = FAILURE;
            break;
        }

        dwRc = pHttpProcess->Input(pMsg, 
                        objSock::GetIPValueByString(pSessNode->m_pSock->cszGetHostIP()), 
                        pSessNode->m_pSock->wGetHostPort());
        if (dwRc != SUCCESS)
        {
            delete pMsg;
        }
    } while (0);

    if (pCondParaNode) DCOP_Free(pCondParaNode);
    if (pReqParaNode) DCOP_Free(pReqParaNode);
}

/*******************************************************
  函 数 名: CHttpJson::ProcRsp
  描    述: 处理JSON响应
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpJson::ProcRsp(objMsg *msg)
{
    /// 消息的控制参数区保存的是会话信息
    DWORD dwCtrlLen = 0;
    IHttpServer::SessionNode *pSessNode = (IHttpServer::SessionNode *)msg->GetCtrl(dwCtrlLen);
    if (!pSessNode || (dwCtrlLen < sizeof(IHttpServer::SessionNode)))
    {
        return;
    }

    /// 获取会话信息中的HTTP句柄
    IHttpHandle *http = pSessNode->m_pHttp;
    if (!http)
    {
        return;
    }

    if (!OnSessionRsp(http, msg))
    {
        return;
    }

    CDString strJson((const char *)http->Content().Buffer(), http->Content().Length());
    if (!strJson.Length())
    {
        return;
    }

    http->Content().Clear();
    const char *jsonpCallback = http->GetSysParam("callback");
    if (jsonpCallback)
    {
        http->Content() << jsonpCallback << "(";
    }

    http->Content() << strJson;

    if (jsonpCallback)
    {
        http->Content() << ")";
    }

    http->SetContentLength(http->Content().Length());
    http->SetStatus(IHttpHandle::STATUS_RESPONSE);
}

/*******************************************************
  函 数 名: CHttpJson::OnSessionRsp
  描    述: 处理会话响应
  输    入: 
  输    出: 
  返    回: true:已收到所有的响应; false:需要继续接收后续响应
  修改记录: 
 *******************************************************/
bool CHttpJson::OnSessionRsp(IHttpHandle *http, objMsg *msg)
{
    /// 解析会话头
    CDArray aSessHeads;
    IObjectMember::GetMsgHead(msg->GetDataBuf(), msg->GetDataLen(), &aSessHeads, 0, 0, 0, 0);
    DCOP_SESSION_HEAD *pSessionHead = (DCOP_SESSION_HEAD *)aSessHeads.Pos(0);
    if (!pSessionHead)
    {
        RspError(http, FAILURE);
        return true;
    }

    /// 获取会话数据
    void *pSessionData = *(void **)(pSessionHead + 1);
    if (!pSessionData)
    {
        RspError(http, FAILURE);
        return true;
    }

    /// 获取响应数据头
    CDArray aRspHeads;
    IObjectMember::GetMsgHead(pSessionData, pSessionHead->m_type.m_valueLen, 0, 0, 0, &aRspHeads, 0);
    if (!aRspHeads.Count())
    {
        RspError(http, FAILURE);
        return true;
    }

    /// 响应数量和头部不符(格式错误)
    if (aRspHeads.Count() != pSessionHead->m_count)
    {
        RspError(http, FAILURE);
        return true;
    }

    /// 响应字段在第一个消息节点中
    DCOP_RESPONSE_HEAD *pRspHead = (DCOP_RESPONSE_HEAD *)aRspHeads.Pos(0);
    if (!pRspHead)
    {
        RspError(http, FAILURE);
        return true;
    }

    /// 失败的响应
    if (pRspHead->m_retCode != SUCCESS)
    {
        RspError(http, pRspHead->m_retCode);
        return true;
    }

    /// 非查询类的响应
    if (!pRspHead->m_recordCount && (pSessionHead->m_ctrl != DCOP_CTRL_GET))
    {
        RspError(http, SUCCESS);
        return true;
    }

    /// 查询数量的响应
    if (!pRspHead->m_paraCount)
    {
        RspCount(http, pRspHead->m_recordCount);
        return true;
    }

    /// 解析响应数据内容
    CDStream sRspPara(pRspHead->m_paraCount * sizeof(DCOP_PARA_NODE));
    DCOP_PARA_NODE *pRspPara = (DCOP_PARA_NODE *)sRspPara.Buffer();
    (void)IObjectMember::GetMsgParaData(*(void **)(pRspHead + 1), pRspHead->m_paraCount, pRspHead->m_paraLen, pRspPara);

    Json::Value root;
    Json::FastWriter writer;

#if _HTTPD_DEBUG_
    PrintLog(STR_FORMAT("<Session(%d) Json RspRecord Start> RspCount:%d RecCount:%d", 
                pSessionHead->m_session, aRspHeads.Count(), pRspHead->m_recordCount), 
                PrintToConsole, 0);
#endif

    /// 将数据内容转换成JSON格式
    Json::Value records;
    if (RspRecord(records, pSessionHead->m_attribute, aRspHeads, pRspPara, pRspHead->m_paraCount) != SUCCESS)
    {
        RspError(http, FAILURE);
        return true;
    }

#if _HTTPD_DEBUG_
    PrintLog(STR_FORMAT("<Session(%d) Json RspRecord End> RspCount:%d RecCount:%d", 
                pSessionHead->m_session, aRspHeads.Count(), pRspHead->m_recordCount), 
                PrintToConsole, 0);
#endif

    /// 将转换后的JSON更新到http内容中
    if (aRspHeads.Count() >= pRspHead->m_recordCount)
    {
        root["rc"] = 0;
        root["count"] = (unsigned int)pRspHead->m_recordCount;
        root["records"] = records;

        http->Content().Clear();
    #if _HTTPD_DEBUG_
        PrintLog(STR_FORMAT("<Session(%d) Json Write Start> RspSingle", 
                pSessionHead->m_session), 
                PrintToConsole, 0);
    #endif
        std::string json_text = writer.write(root);
    #if _HTTPD_DEBUG_
        PrintLog(STR_FORMAT("<Session(%d) Json Write End> RspSingle", 
                pSessionHead->m_session), 
                PrintToConsole, 0);
    #endif
        http->Content() << json_text.c_str();

        return true;
    }

    /// 不是一次响应所有的记录，需要保存到http句柄中(需要加锁保护)
    AutoObjLock(this);

#if _HTTPD_DEBUG_
    PrintLog(STR_FORMAT("<Session(%d) Json Parse Start> RspCount:%d RecCount:%d", 
                pSessionHead->m_session, aRspHeads.Count(), pRspHead->m_recordCount), 
                PrintToConsole, 0);
#endif

    const char *jsonBuf = (const char *)http->Content().Buffer();
    DWORD jsonLen = http->Content().Length();
    if (jsonBuf && jsonLen)
    {
        Json::Reader reader;
        (void)reader.parse(jsonBuf, jsonBuf + jsonLen, root, false);
        for (DWORD i = 0; i < records.size(); ++i)
        {
            root["records"].append(records[i]);
        }
    }
    else
    {
        root["rc"] = 0;
        root["count"] = (unsigned int)pRspHead->m_recordCount;
        root["records"] = records;
    }

#if _HTTPD_DEBUG_
    PrintLog(STR_FORMAT("<Session(%d) Json Parse End> RspCount:%d RecCount:%d", 
                pSessionHead->m_session, aRspHeads.Count(), pRspHead->m_recordCount), 
                PrintToConsole, 0);
#endif

    http->Content().Clear();
#if _HTTPD_DEBUG_
    PrintLog(STR_FORMAT("<Session(%d) Json Write Start> RspCount:%d RecCount:%d", 
                pSessionHead->m_session, aRspHeads.Count(), pRspHead->m_recordCount), 
                PrintToConsole, 0);
#endif
    std::string json_text = writer.write(root);
#if _HTTPD_DEBUG_
    PrintLog(STR_FORMAT("<Session(%d) Json Write End> RspCount:%d RecCount:%d", 
                pSessionHead->m_session, aRspHeads.Count(), pRspHead->m_recordCount), 
                PrintToConsole, 0);
#endif
    http->Content() << json_text.c_str();

    if (root["records"].size() >= pRspHead->m_recordCount)
    {
        return true;
    }

    return false;
}

/*******************************************************
  函 数 名: CHttpJson::RspError
  描    述: 响应错误码
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpJson::RspError(IHttpHandle *http, DWORD dwErrCode)
{
    Json::Value root;
    Json::FastWriter writer;

    root["rc"] = (unsigned int)dwErrCode;

    http->Content().Clear();
    std::string json_text = writer.write(root);
    http->Content() << json_text.c_str();
}

/*******************************************************
  函 数 名: CHttpJson::RspCount
  描    述: 响应数量值
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpJson::RspCount(IHttpHandle *http, DWORD dwRecCount)
{
    Json::Value root;
    Json::FastWriter writer;

    root["rc"] = 0;
    root["count"] = (unsigned int)dwRecCount;

    http->Content().Clear();
    std::string json_text = writer.write(root);
    http->Content() << json_text.c_str();
}

/*******************************************************
  函 数 名: CHttpJson::RspRecord
  描    述: 响应记录值
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CHttpJson::RspRecord(Json::Value &result, DWORD dwAttrID, 
                        const CDArray &aRspHeads, DCOP_PARA_NODE *pPara, DWORD dwParaCount)
{
    CHttpProcess *pHttpProcess = (CHttpProcess *)Parent();
    if (!pHttpProcess)
    {
        return FAILURE;
    }

    IModel *piModel = pHttpProcess->GetModel();
    if (!piModel)
    {
        return FAILURE;
    }

    DWORD dwFieldCount = 0;
    IModel::Field *pField = piModel->GetFields(dwAttrID, dwFieldCount);
    if (!pField || !dwFieldCount)
    {
        return FAILURE;
    }

    for (DWORD i = 0; i < aRspHeads.Count(); ++i)
    {
        DCOP_MSG_HEAD *pMsgHead = (DCOP_MSG_HEAD *)aRspHeads.Pos(i);
        if (!pMsgHead) return FAILURE;

        DWORD dwMsgParaCount = 0;
        DWORD dwMsgDataLen = 0;
        if (pMsgHead->m_headType == DCOP_MSG_HEAD_RESPONSE)
        {
            DCOP_RESPONSE_HEAD *pRspHead = (DCOP_RESPONSE_HEAD *)pMsgHead;
            dwMsgParaCount = pRspHead->m_paraCount;
            dwMsgDataLen = pRspHead->m_paraLen;
        }
        else if (pMsgHead->m_headType == DCOP_MSG_HEAD_EVENT)
        {
            DCOP_EVENT_HEAD *pEvtHead = (DCOP_EVENT_HEAD *)pMsgHead;
            dwMsgParaCount = pEvtHead->m_paraCount;
            dwMsgDataLen = pEvtHead->m_paraLen;
        }
        else
        {
            return FAILURE;
        }

        void *pMsgData = IObjectMember::GetMsgParaData(
                        *(void **)((BYTE *)pMsgHead + pMsgHead->m_headSize), 
                        dwMsgParaCount, dwMsgDataLen);
        if (!pMsgData) return FAILURE;

        Json::Value record;
        DWORD dwRc = RspField(record, piModel, dwAttrID, pField, dwFieldCount, pPara, dwParaCount, pMsgData, dwMsgDataLen);
        if (dwRc != SUCCESS) return dwRc;

        result.append(record);
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CHttpJson::RspField
  描    述: 响应字段值
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CHttpJson::RspField(Json::Value &result, IModel *piModel, DWORD dwAttrID, 
                        IModel::Field *pField, DWORD dwFieldCount, 
                        DCOP_PARA_NODE *pPara, DWORD dwParaCount, 
                        void *pData, DWORD dwDataLen)
{
    if (!pPara || !dwParaCount || !pData || !dwDataLen) return FAILURE;

    BYTE *pbyRec = (BYTE *)pData;
    DWORD dwOffset = 0;
    for (DWORD i = 0; i < dwParaCount; ++i)
    {
        const char *pFieldName = NULL;
        DWORD dwFieldID = pPara[i].m_paraID;
        if (DCOP_SPECPARA(dwFieldID, DCOP_FIELD_RELATION))
        {
            pFieldName = CCommand::GetRelFieldName(piModel, dwAttrID, dwFieldID, dwFieldCount);
        }
        else
        {
            if (!dwFieldID || (dwFieldID > dwFieldCount))
            {
                dwOffset += pPara[i].m_paraSize;
                if (dwOffset >= dwDataLen)
                {
                    break;
                }

                continue;
            }

            pFieldName = pField[dwFieldID - 1].m_fieldName;
        }

        if (!pFieldName)
        {
            dwOffset += pPara[i].m_paraSize;
            if (dwOffset >= dwDataLen)
            {
                break;
            }

            continue;
        }

        switch (pPara[i].m_paraType)
        {
            case IModel::FIELD_BYTE:
            case IModel::FIELD_WORD:
            case IModel::FIELD_DWORD:
            case IModel::FIELD_IDENTIFY:
            case IModel::FIELD_NUMBER:
                result[pFieldName] = (unsigned int)Bytes_GetDwordValue(pbyRec + dwOffset, pPara[i].m_paraSize);
                break;
            case IModel::FIELD_SHORT:
            case IModel::FIELD_INTEGER:
                result[pFieldName] = (int)Bytes_GetDwordValue(pbyRec + dwOffset, pPara[i].m_paraSize);
                break;
            case IModel::FIELD_CHAR:
            case IModel::FIELD_STRING:
            {
                CDString strTmp((char *)pbyRec + dwOffset, pPara[i].m_paraSize);
                result[pFieldName] = (const char *)strTmp;
            }
                break;
            case IModel::FIELD_BUFFER:
                result[pFieldName] = (const char *)CBufferString(pbyRec + dwOffset, pPara[i].m_paraSize);
                break;
            case IModel::FIELD_DATE:
                break;
            case IModel::FIELD_TIME:
                break;
            case IModel::FIELD_IP:
            {
                char szIP[OSSOCK_IPSIZE];
                (void)memset(szIP, 0, sizeof(szIP));
                objSock::GetIPStringByValue(*(DWORD *)(pbyRec + dwOffset), szIP);
                result[pFieldName] = szIP;
            }
                break;
            case IModel::FIELD_PTR:
                result[pFieldName] = (const char *)STR_FORMAT("%p", *(void **)(pbyRec + dwOffset));
                break;
            case IModel::FIELD_TIMER:
            {
                CDString strTimer;
                ITimer::IWheel::GetString((ITimer::Handle)(pbyRec + dwOffset), strTimer);
                result[pFieldName] = (const char *)strTimer;
            }
                break;
            case IModel::FIELD_PASS:
                result[pFieldName] = "********";
                break;
            default:
                break;
        }

        dwOffset += pPara[i].m_paraSize;
        if (dwOffset >= dwDataLen)
        {
            break;
        }
    }

    return SUCCESS;
}

