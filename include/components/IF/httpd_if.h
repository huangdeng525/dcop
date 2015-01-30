/// -------------------------------------------------
/// httpd_if.h : 超文本接入公共头文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#ifndef _HTTPD_IF_H_
#define _HTTPD_IF_H_

#include "Object_if.h"
#include "ObjModel_if.h"
#include "user_if.h"
#include "sock.h"
#include "stream/dstream.h"


/////////////////////////////////////////////////////
///                'HTTP句柄'说明
/// -------------------------------------------------
/// 'HTTP句柄'(IHttpHandle)用于HTTP:
///     从 '请求' -> '处理' -> '响应' 过程中
///     整个数据的保存。
/////////////////////////////////////////////////////


/// 创建HTTP句柄的宏
#define CREATE_HTTP_HANDLE() \
    IHttpHandle::CreateInstance(__FILE__, __LINE__)


/// -------------------------------------------------
/// 超文本句柄对象接口类
/// -------------------------------------------------
INTF_VER(IHttpHandle, 1, 0, 0);
class IHttpHandle : public Instance
{
public:
    static IHttpHandle *CreateInstance(const char *file, int line);

public:
    /// 请求方法(数值)
    enum METHOD
    {
        METHOD_GET = 0,                     // 请求获取Request-URI所标识的资源
        METHOD_POST,                        // 在Request-URI所标识的资源后附加新的数据
        METHOD_HEAD,                        // 请求获取由Request-URI所标识的资源的响应消息报头
        METHOD_PUT,                         // 请求服务器存储一个资源，并用Request-URI作为其标识
        METHOD_DELETE,                      // 请求服务器删除Request-URI所标识的资源
        METHOD_TRACE,                       // 请求服务器回送收到的请求信息，主要用于测试或诊断
        METHOD_CONNECT,                     // 保留将来使用
        METHOD_OPTIONS                      // 请求查询服务器的性能，或者查询与资源相关的选项和需求
    };

    /// 协议版本(数值)
    enum PROTOCOL
    {
        PROTO_VER_1_1 = 0,                  // HTTP/1.1
        PROTO_VER_1_0                       // HTTP/1.0
    };

    /// 状态码(数值)
    enum STATUS
    {
        STATUS_UNSTART = 0,
        STATUS_REQUEST,
        STATUS_PROCESS,
        STATUS_JSON,
        STATUS_RESPONSE,

        STATUS_OK,                          // Ok : 客户端请求成功
        STATUS_BAD_REQUEST,                 // Bad Request : 客户端请求有语法错误，不能被服务器所理解
        STATUS_UNAUTHORIZED,                // Unauthorized : 请求未经授权，这个状态代码必须和WWW-Authenticate报头域一起使用
        STATUS_FORBIDDEN,                   // Forbidden : 服务器收到请求，但是拒绝提供服务
        STATUS_NOT_FOUND,                   // Not Found : 请求资源不存在，eg：输入了错误的URL
        STATUS_INTERNAL_SERVER_ERROR,       // Internal Server Error : 服务器发生不可预期的错误
        STATUS_SERVER_UNAVAILABLE,          // Server Unavailable : 服务器当前不能处理客户端的请求，一段时间后可能恢复正常
    };

    /// MIME(数值)
    enum MIME
    {
        MIME_TEXT_HTML = 0,
    };

    /// 内容压缩(数值)
    enum COMPRESS
    {
        COMPRESS_NO = 0,
        COMPRESS_GZIP,
        COMPRESS_DEFLATE
    };

public:
    /// 是否完整
    virtual bool bCompleted() = 0;

    /// 是否动态请求
    virtual bool bDynamic() = 0;

    /// 设置和获取请求方法
    virtual void SetMethod(METHOD method) = 0;
    virtual METHOD GetMethod() = 0;

    /// 设置和获取请求URI路径
    virtual void SetURI(const char *URI) = 0;
    virtual const char *GetURI() = 0;

    /// 设置和获取协议版本
    virtual void SetProtocol(PROTOCOL protocol) = 0;
    virtual PROTOCOL GetProtocol() = 0;

    /// 设置和获取获取状态码
    virtual void SetStatus(STATUS status) = 0;
    virtual STATUS GetStatus() = 0;

    /// 设置和获取连接状态
    virtual void SetConnection(bool keep) = 0;
    virtual bool GetConnection() = 0;

    /// 设置和获取主机域名
    virtual void SetHost(const char *HOST) = 0;
    virtual const char *GetHost() = 0;

    /// 设置和获取可接受类型
    virtual void SetAccept(const char *accept) = 0;
    virtual const char *GetAccept() = 0;

    /// 设置和获取可接受字符集
    virtual void SetAcceptCharset(const char *accept_charset) = 0;
    virtual const char *GetAcceptCharset() = 0;

    /// 设置和获取可接受编码
    virtual void SetAcceptEncoding(const char *accept_encoding) = 0;
    virtual const char *GetAcceptEncoding() = 0;

    /// 设置和获取可接受语言
    virtual void SetAcceptLanguage(const char *accept_language) = 0;
    virtual const char *GetAcceptLanguage() = 0;

    /// 设置和获取权限证明
    virtual void SetAuthorization(const char *authorization) = 0;
    virtual const char *GetAuthorization() = 0;

    /// 设置和获取终端信息
    virtual void SetUserAgent(const char *user_agent) = 0;
    virtual const char *GetUserAgent() = 0;

    /// 引用内容对象
    virtual CDStream &Content() = 0;

    /// 设置和获取内容类型
    virtual void SetContentType(MIME content_type) = 0;
    virtual MIME GetContentType() = 0;

    /// 设置和获取内容长度
    virtual void SetContentLength(DWORD content_length) = 0;
    virtual DWORD GetContentLength() = 0;

    /// 设置和获取内容编码
    virtual void SetContentEncoding(COMPRESS content_encoding) = 0;
    virtual COMPRESS GetContentEncoding() = 0;

    /// 设置和获取内容语言
    virtual void SetContentLanguage(const char *content_language) = 0;
    virtual const char *GetContentLanguage() = 0;

    /// 设置和获取最后修改时间
    virtual void SetLastModified(const char *last_modified) = 0;
    virtual const char *GetLastModified() = 0;

    /// 设置和获取到期时间
    virtual void SetExpires(const char *expires) = 0;
    virtual const char *GetExpires() = 0;

    /// 解析和获取参数
    virtual void ParseParams() = 0;
    virtual const char *GetSysParam(const char *param_name) = 0;
    virtual CDArray &GetReqParamList() = 0;
    virtual CDArray &GetCondParamList() = 0;
    virtual BYTE GetCondition() = 0;

    /// 使能动态请求
    virtual void EnableDynamic() = 0;
};


/// -------------------------------------------------
/// 超文本处理对象接口类
/// -------------------------------------------------
INTF_VER(IHttpProcess, 1, 0, 0);
interface IHttpProcess : public IObject
{
    /// 资源路径节点
    struct ResPathNode
    {
        const char *m_resPath;                  // 资源路径
        DWORD m_attrID;                         // 属性ID
    };

    /// 资源类型节点
    struct ResTypeNode
    {
        const char *m_resType;                  // 资源类型
        IObject *m_objProc;                     // 处理对象
    };

    /// 注册资源路径和属性值
    virtual DWORD RegResPath(ResPathNode *pNode, DWORD dwCount) = 0;

    /// 注册资源类型和处理对象
    virtual DWORD RegResType(ResTypeNode *pNode, DWORD dwCount) = 0;

    /// 获取资源路径所属对象
    virtual IObject *GetResOwner(const char *URI, DWORD &rdwAttrID) = 0;

    /// 输入消息(继续向内部对象分发)
    virtual DWORD Input(objMsg *msg, DWORD clientIP, WORD clientPort) = 0;
};


/// -------------------------------------------------
/// 超文本接入接口定义
/// -------------------------------------------------
INTF_VER(IHttpServer, 1, 0, 0);
interface IHttpServer : public IObject
{
    /// 会话节点
    struct SessionNode
    {
        char m_szUserName[IUser::NAME_SIZE];
        DWORD m_dwUserID;
        DWORD m_dwUserGroup;
        DWORD m_dwSessID;
        objSock *m_pSock;
        IHttpHandle *m_pHttp;
        bool m_bSetClientInfo;
    };

    /// 保存http句柄到会话节点中
    virtual void SaveHttpToSession(
                        DWORD dwSessionID, 
                        IHttpHandle *pHttp
                        ) = 0;
};


#endif // #ifndef _HTTPD_IF_H_

