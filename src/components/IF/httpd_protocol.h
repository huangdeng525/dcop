/// -------------------------------------------------
/// httpd_protocol.h : 超文本协议私有头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _HTTPD_PROTOCOL_H_
#define _HTTPD_PROTOCOL_H_

#include "BaseClass.h"
#include "httpd_handle.h"
#include "stream/dstream.h"


/// -------------------------------------------------
/// 超文本请求对象接口类
/// -------------------------------------------------
INTF_VER(IHttpRequest, 1, 0, 0);
interface IHttpRequest : public Instance
{
    /// 输入请求
    virtual void Input(IHttpHandle *http, const char *req) = 0;
};


/// -------------------------------------------------
/// 超文本响应对象接口类
/// -------------------------------------------------
INTF_VER(IHttpResponse, 1, 0, 0);
interface IHttpResponse : public Instance
{
    /// 输出响应
    virtual void Output(IHttpHandle *http, CDStream &rsp, DWORD *pdwHeadSize = 0) = 0;
};


/// -------------------------------------------------
/// 超文本请求对象实现类
/// -------------------------------------------------
class CHttpRequest : public IHttpRequest
{
public:
    /// 命令组成定义
    enum HEAD_LINE
    {
        HEAD_LINE_METHOD = 0,
        HEAD_LINE_URI,
        HEAD_LINE_PROTOCOL,
    };

    /// 请求方法(文本)
    static const char *Method[];

    /// 协议版本(文本)
    static const char *Protocol[];

public:
    static void StrDecode(char *str);
    static int hexit(char c);

public:
    CHttpRequest(Instance *piParent, int argc, char **argv);
    ~CHttpRequest();

    DCOP_DECLARE_INSTANCE;

    void Input(IHttpHandle *http, const char *req);

private:
    void Analyze(IHttpHandle *http, const char *req);
    DWORD GetHeadLine(IHttpHandle *http, const char *head);
    DWORD GetMethod(const char *head, IHttpHandle::METHOD &method);
    DWORD GetProtocol(const char *head, IHttpHandle::PROTOCOL &protocol);
    void GetHeadItem(IHttpHandle *http, CDString *head);
};


/// -------------------------------------------------
/// 超文本响应对象实现类
/// -------------------------------------------------
class CHttpResponse : public IHttpResponse
{
public:
    /// 协议版本(服务器支持的最高版本)
    static const char *Protocol;

    /// 状态码(文本)
    static const char *Status[];

    /// 服务器信息
    static const char *Server;

    /// HTTP已经规定了使用RFC1123时间格式
    static const char *RFC1123FMT;

    /// MIME(文本)
    static const char *MIME[];

    /// 内容压缩(文本)
    static const char *Compress[];

public:
    static const char *GetStatus(IHttpHandle *http);
    static const char *GetContentType(IHttpHandle *http);
    static const char *GetContentType(const char *name);

public:
    CHttpResponse(Instance *piParent, int argc, char **argv);
    ~CHttpResponse();

    DCOP_DECLARE_INSTANCE;

    void Output(IHttpHandle *http, CDStream &rsp, DWORD *pdwHeadSize);

private:
    void GetHeadItem(IHttpHandle *http, CDStream &rsp);
    bool IsCompressSupport(IHttpHandle *http, IHttpHandle::COMPRESS compress);
    void DeflateContent(IHttpHandle *http);
    void GzipContent(IHttpHandle *http);
};


#endif // #ifndef _HTTPD_PROTOCOL_H_

