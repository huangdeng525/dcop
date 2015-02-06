/// -------------------------------------------------
/// httpd_handle.h : 超文本句柄私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _HTTPD_HANDLE_H_
#define _HTTPD_HANDLE_H_

#include "httpd_if.h"
#include "array/darray.h"


/// -------------------------------------------------
/// 超文本句柄对象实现类
/// -------------------------------------------------
class CHttpHandle : public IHttpHandle
{
public:
    /// 参数顺序
    enum
    {
        HTTP_PARAM_REQ = 0,
        HTTP_PARAM_COND,
        HTTP_PARAM_SYS
    };

public:
    CHttpHandle(Instance *piParent, int argc, char **argv);
    ~CHttpHandle();

    DCOP_DECLARE_INSTANCE;

    bool bCompleted();
    bool bDynamic() {return m_dynamic;}

    void SetMethod(METHOD method) {m_method = method;}
    METHOD GetMethod() {return m_method;}

    void SetURI(const char *URI) {m_URI = URI;}
    const char *GetURI() {return (const char *)m_URI;}

    void SetProtocol(PROTOCOL protocol) {m_protocol = protocol;}
    PROTOCOL GetProtocol() {return m_protocol;}

    void SetStatus(STATUS status) {m_status = status;}
    STATUS GetStatus() {return m_status;}

    void SetConnection(bool keep) {m_keep = keep;}
    bool GetConnection() {return m_keep;}

    void SetHost(const char *HOST) {m_HOST = HOST;}
    const char *GetHost() {return m_HOST;}

    void SetAccept(const char *accept) {m_accept = accept;}
    const char *GetAccept() {return m_accept;}

    void SetAcceptCharset(const char *accept_charset) {m_accept_charset = accept_charset;}
    const char *GetAcceptCharset() {return m_accept_charset;}

    void SetAcceptEncoding(const char *accept_encoding) {m_accept_encoding = accept_encoding;}
    const char *GetAcceptEncoding() {return m_accept_encoding;}

    void SetAcceptLanguage(const char *accept_language) {m_accept_language = accept_language;}
    const char *GetAcceptLanguage() {return m_accept_language;}

    void SetAuthorization(const char *authorization) {m_authorization = authorization;}
    const char *GetAuthorization() {return m_authorization;}

    void SetUserAgent(const char *user_agent) {m_user_agent = user_agent;}
    const char *GetUserAgent() {return m_user_agent;}

    CDStream &Content() {return m_content;}

    void SetContentType(MIME content_type) {m_content_type = content_type;}
    MIME GetContentType() {return m_content_type;}

    void SetContentLength(DWORD content_length) {m_content_length = content_length;}
    DWORD GetContentLength() {return m_content_length;}

    void SetContentEncoding(COMPRESS content_encoding) {m_content_encoding = content_encoding;}
    COMPRESS GetContentEncoding() {return m_content_encoding;}

    void SetContentLanguage(const char *content_language) {m_content_language = content_language;}
    const char *GetContentLanguage() {return m_content_language;}

    void SetLastModified(const char *last_modified) {m_last_modified = last_modified;}
    const char *GetLastModified() {return m_last_modified;}

    void SetExpires(const char *expires) {m_expires = expires;}
    const char *GetExpires() {return m_expires;}

    void ParseParams();
    const char *GetSysParam(const char *param_name);
    CDArray &GetReqParamList() {return m_req_param_list;}
    CDArray &GetCondParamList() {return m_cond_param_list;}
    BYTE GetCondition() {return m_condition;}

    void EnableDynamic() {m_dynamic = true;}

private:
    PROTOCOL m_protocol;
    METHOD m_method;
    CDString m_URI;

    bool m_dynamic;
    STATUS m_status;
    bool m_keep;

    CDString m_HOST;
    CDString m_accept;
    CDString m_accept_charset;
    CDString m_accept_encoding;
    CDString m_accept_language;
    CDString m_authorization;
    CDString m_user_agent;

    CDStream m_content;
    MIME m_content_type;
    DWORD m_content_length;
    COMPRESS m_content_encoding;
    CDString m_content_language;

    CDString m_last_modified;
    CDString m_expires;

    CDString m_params;
    CDString m_sys_params;
    CDString m_req_params;
    CDString m_cond_params;
    CDArray m_sys_param_list;
    CDArray m_req_param_list;
    CDArray m_cond_param_list;
    BYTE m_condition;
};


#endif // #ifndef _HTTPD_HANDLE_H_

