/// -------------------------------------------------
/// httpd_json.h : 超文本处理JSON私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _HTTPD_JSON_H_
#define _HTTPD_JSON_H_

#include "httpd_restful.h"
#include "BaseMessage.h"


/////////////////////////////////////////////////////
/// 'JSON' : 支持处理json类型的请求
/// -------------------------------------------------
/// 'JSONP': 支持处理json回调的请求
/////////////////////////////////////////////////////


/// -------------------------------------------------
/// 超文本处理JSON对象实现类
/// -------------------------------------------------
class CHttpJson : public IObject
{
public:
    CHttpJson(Instance *piParent, int argc, char **argv);
    ~CHttpJson();

    DCOP_DECLARE_INSTANCE;
    DCOP_DECLARE_IOBJECT;

    void Proc(objMsg *msg);

private:
    void ProcReq(objMsg *msg);
    void ProcRsp(objMsg *msg);

    bool OnSessionRsp(IHttpHandle *http, objMsg *msg);

    void RspError(IHttpHandle *http, DWORD dwErrCode);
    void RspCount(IHttpHandle *http, DWORD dwRecCount);

    DWORD RspRecord(Json::Value &result, 
                DWORD dwAttrID, 
                const CDArray &aRspHeads, 
                DCOP_PARA_NODE *pPara, 
                DWORD dwParaCount);

    DWORD RspField(Json::Value &result, 
                IModel *piModel, 
                DWORD dwAttrID, 
                IModel::Field *pField, 
                DWORD dwFieldCount, 
                DCOP_PARA_NODE *pPara, 
                DWORD dwParaCount, 
                void *pData, 
                DWORD dwDataLen);
};


#endif // #ifndef _HTTPD_JSON_H_

