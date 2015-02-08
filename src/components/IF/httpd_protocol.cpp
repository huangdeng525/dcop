/// -------------------------------------------------
/// httpd_protocol.cpp : 超文本协议实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "httpd_protocol.h"
#include "Factory_if.h"
#include "zlib.h"
#include <time.h>
#include <ctype.h>


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CHttpRequest, "HttpRequest")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CHttpRequest)
    DCOP_IMPLEMENT_INTERFACE(IHttpRequest)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 请求方法(文本)
/// -------------------------------------------------
const char *CHttpRequest::Method[] = 
{
    "GET",
    "POST",
    "HEAD",
    "PUT",
    "DELETE",
    "TRACE",
    "CONNECT",
    "OPTIONS"
};

/// -------------------------------------------------
/// 协议版本(文本)
/// -------------------------------------------------
const char *CHttpRequest::Protocol[] = 
{
    "HTTP/1.1",
    "HTTP/1.0"
};


/*******************************************************
  函 数 名: CHttpRequest::CHttpRequest
  描    述: CHttpRequest构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CHttpRequest::CHttpRequest(Instance *piParent, int argc, char **argv)
{
    DCOP_CONSTRUCT_INSTANCE(piParent);
}

/*******************************************************
  函 数 名: CHttpRequest::~CHttpRequest
  描    述: CHttpRequest析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CHttpRequest::~CHttpRequest()
{
    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CHttpRequest::Input
  描    述: 输入请求
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpRequest::Input(IHttpHandle *http, const char *req)
{
    if (!http || !req)
    {
        return;
    }

    if (http->GetStatus() > IHttpHandle::STATUS_REQUEST)
    {
        return;
    }

    http->Content() << req;

    Analyze(http, CDString((const char *)http->Content().Buffer(), http->Content().Length()));
}

/*******************************************************
  函 数 名: CHttpRequest::Analyze
  描    述: 解析文本
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpRequest::Analyze(IHttpHandle *http, const char *req)
{
    if (!http || !req) return;

    /// 已经开始解析，说明是等待请求内容
    if (http->GetStatus() != IHttpHandle::STATUS_UNSTART)
    {
        if (http->bCompleted())
        {
            /// 内容已完整，下一步该进行处理了
            http->SetStatus(IHttpHandle::STATUS_PROCESS);
        }

        return;
    }

    /// 下面是未开始解析，先找'空行'
    const char *blank = strstr(req, "\r\n\r\n");

    /// 找不到空行，继续等待下次输入
    if (!blank)
    {
        return;
    }

    /// 空行在开始，说明输入格式错误
    if (blank <= req)
    {
        http->SetStatus(IHttpHandle::STATUS_BAD_REQUEST);
        return;
    }

    DWORD headLen = (DWORD)(blank - req);

    /// 找到'空行'后，'空行'前是'请求行'+'消息头'
    CDString strHead(req, headLen);
    if (!strHead.Length())
    {
        return;
    }

    CDArray strList;
    strHead.Split("\n", strList);
    if (!strList.Count())
    {
        return;
    }

    /// 第一行是'请求行'
    CDString *pLineFirst = (CDString *)strList.Pos(0);
    if (!pLineFirst) return;
    DWORD dwRc = GetHeadLine(http, *pLineFirst);
    if (dwRc != SUCCESS)
    {
        http->SetStatus(IHttpHandle::STATUS_BAD_REQUEST);
        return;
    }

    /// 接着是多个'请求头'
    for (DWORD i = 1; i < strList.Count(); ++i)
    {
        GetHeadItem(http, (CDString *)strList.Pos(i));
    }

    /// '空行'之前的信息已经解析，需要去掉
    dwRc = http->Content().Remove(0, headLen + 4);
    if (dwRc != SUCCESS)
    {
        http->SetStatus(IHttpHandle::STATUS_BAD_REQUEST);
        return;
    }

    /// 解析参数
    http->ParseParams();

    /// 成功解析后，设置为继续等待请求状态
    http->SetStatus(IHttpHandle::STATUS_REQUEST);

    /// 内容已完整，下一步该进行处理了
    if (http->bCompleted())
    {
        http->SetStatus(IHttpHandle::STATUS_PROCESS);
    }
}

/*******************************************************
  函 数 名: CHttpRequest::GetHeadLine
  描    述: 获取行头
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CHttpRequest::GetHeadLine(IHttpHandle *http, const char *head)
{
    if (!http || !head) return FAILURE;
    DWORD dwRc = SUCCESS;

    CDString str(head);
    if (!str.Length())
    {
        return FAILURE;
    }

    str.TrimRight("\r\n");
    if (str.Get(0) == ' ')
    {
        return FAILURE;
    }

    CDArray strList;
    str.Split(" ", strList);
    if (!strList.Count())
    {
        return FAILURE;
    }

    /// 获取请求方法
    CDString *pStr = (CDString *)strList.Pos(HEAD_LINE_METHOD);
    if (!pStr || !pStr->Length()) return FAILURE;
    IHttpHandle::METHOD method = IHttpHandle::METHOD_GET;
    dwRc = GetMethod(*pStr, method);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    http->SetMethod(method);

    /// 获取URI
    pStr = (CDString *)strList.Pos(HEAD_LINE_URI);
    if (!pStr || !pStr->Length()) return FAILURE;
    http->SetURI(*pStr);

    /// 获取协议版本
    pStr = (CDString *)strList.Pos(HEAD_LINE_PROTOCOL);
    if (!pStr || !pStr->Length()) return FAILURE;
    IHttpHandle::PROTOCOL protocol = IHttpHandle::PROTO_VER_1_1;
    dwRc = GetProtocol(*pStr, protocol);
    if (dwRc != SUCCESS)
    {
        return dwRc;
    }

    http->SetProtocol(protocol);

    return SUCCESS;
}

/*******************************************************
  函 数 名: CHttpRequest::GetMethod
  描    述: 获取方法
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CHttpRequest::GetMethod(const char *head, IHttpHandle::METHOD &method)
{
    if (!head) return FAILURE;

    for (DWORD i = 0; i < ARRAY_SIZE(Method); ++i)
    {
        if (!stricmp(Method[i], head))
        {
            method = (IHttpHandle::METHOD)i;
            return SUCCESS;
        }
    }

    return FAILURE;
}

/*******************************************************
  函 数 名: CHttpRequest::GetProtocol
  描    述: 获取协议版本
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CHttpRequest::GetProtocol(const char *head, IHttpHandle::PROTOCOL &protocol)
{
    if (!head) return FAILURE;

    for (DWORD i = 0; i < ARRAY_SIZE(Protocol); ++i)
    {
        if (!stricmp(Protocol[i], head))
        {
            protocol = (IHttpHandle::PROTOCOL)i;
            return SUCCESS;
        }
    }

    return FAILURE;
}

/*******************************************************
  函 数 名: CHttpRequest::GetHeadItem
  描    述: 获取消息报头
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpRequest::GetHeadItem(IHttpHandle *http, CDString *head)
{
    if (!http || !head) return;

    CDArray strList;
    head->Split(":", strList);
    if (!strList.Count())
    {
        return;
    }

    CDString *pStrName = (CDString *)strList.Pos(0);
    if (!pStrName)
    {
        return;
    }

    CDString *pStrValue = (CDString *)strList.Pos(1);
    if (!pStrValue)
    {
        return;
    }

    pStrName->Trim(" \t\r\n");
    pStrValue->Trim(" \t\r\n");

    if (!stricmp(*pStrName, "Connection"))
    {
        /// HTTP/1.1默认都是Keep-Alive，所以只判断是否是Close
        http->SetConnection((!stricmp(*pStrValue, "Close"))? false : true);
        return;
    }

    if (!stricmp(*pStrName, "Host"))
    {
        http->SetHost(*pStrValue);
        return;
    }

    if (!stricmp(*pStrName, "Accept"))
    {
        http->SetAccept(*pStrValue);
        return;
    }

    if (!stricmp(*pStrName, "Accept-Charset"))
    {
        http->SetAcceptCharset(*pStrValue);
        return;
    }

    if (!stricmp(*pStrName, "Accept-Encoding"))
    {
        http->SetAcceptEncoding(*pStrValue);
        return;
    }

    if (!stricmp(*pStrName, "Accept-Language"))
    {
        http->SetAcceptLanguage(*pStrValue);
        return;
    }

    if (!stricmp(*pStrName, "Authorization"))
    {
        http->SetAuthorization(*pStrValue);
        return;
    }

    if (!stricmp(*pStrName, "User-Agent"))
    {
        http->SetUserAgent(*pStrValue);
        return;
    }
}

/*******************************************************
  函 数 名: CHttpRequest::StrDecode
  描    述: 字符串解码 (比如'%3C'->'<')
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpRequest::StrDecode(char *str)
{
    if (!str) return;

    char *to = str;
    char *from = str;

    for (; *from != '\0'; ++to, ++from)
    {
        if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2]))
        {
            *to = hexit(from[1]) * 16 + hexit(from[2]);
            from += 2;
        }
        else
        {
            *to = *from;
        }
    }

    *to = '\0';
}

/*******************************************************
  函 数 名: CHttpRequest::hexit
  描    述: 16进制字符到数字的转换
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
int CHttpRequest::hexit(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return 0;
}


/// -------------------------------------------------
/// 实现类厂
/// -------------------------------------------------
DCOP_IMPLEMENT_FACTORY(CHttpResponse, "HttpResponse")

/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CHttpResponse)
    DCOP_IMPLEMENT_INTERFACE(IHttpResponse)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END

/// -------------------------------------------------
/// 协议版本(服务器支持的最高版本)
/// -------------------------------------------------
const char *CHttpResponse::Protocol = "HTTP/1.1";

/// -------------------------------------------------
/// 状态码(文本)
/// -------------------------------------------------
const char *CHttpResponse::Status[] = 
{
    "",
    "",
    "",
    "",
    "",
    "200 OK",
    "400 Bad Request",
    "401 Unauthorized",
    "403 Forbidden",
    "404 Not Found",
    "500 Internal Server Error",
    "503 Server Unavailable"
};

/// -------------------------------------------------
/// 服务器信息
/// -------------------------------------------------
const char *CHttpResponse::Server = "DCOServer/0.1";

/// -------------------------------------------------
/// HTTP已经规定了使用RFC1123时间格式
/// -------------------------------------------------
const char *CHttpResponse::RFC1123FMT = "%a, %d %b %Y %H:%M:%S GMT";

/// -------------------------------------------------
/// MIME(文本)
/// -------------------------------------------------
const char *CHttpResponse::MIME[] = 
{
    "text/html",
};

/// -------------------------------------------------
/// 内容压缩(文本)
/// -------------------------------------------------
const char *CHttpResponse::Compress[] = 
{
    "",
    "gzip",
    "deflate"
};


/*******************************************************
  函 数 名: CHttpResponse::CHttpCHttpResponseRequest
  描    述: CHttpResponse构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CHttpResponse::CHttpResponse(Instance *piParent, int argc, char **argv)
{
    DCOP_CONSTRUCT_INSTANCE(piParent);

}

/*******************************************************
  函 数 名: CHttpResponse::~CHttpResponse
  描    述: CHttpResponse析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CHttpResponse::~CHttpResponse()
{

    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CHttpResponse::Output
  描    述: 输出响应
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpResponse::Output(IHttpHandle *http, CDStream &rsp, DWORD *pdwHeadSize)
{
    if (!http) return;

    /// 尝试压缩动态内容
    /// if (http->bDynamic())
    /// 先只要是完整的就进行压缩
    if (http->bCompleted())
    {
    #if _HTTPD_DEBUG_
        PrintLog(STR_FORMAT("<Output Compress Start> Status:%d", 
                http->GetStatus()), 
                PrintToConsole, 0);
    #endif
        if (IsCompressSupport(http, IHttpHandle::COMPRESS_DEFLATE))
        {
            DeflateContent(http);
        }
        else if (IsCompressSupport(http, IHttpHandle::COMPRESS_GZIP))
        {
            GzipContent(http);
        }
        else
        {
        }
    #if _HTTPD_DEBUG_
        PrintLog(STR_FORMAT("<Output Compress End> Status:%d", 
                http->GetStatus()), 
                PrintToConsole, 0);
    #endif
    }

    /// 走到响应是正常状态
    if (http->GetStatus() == IHttpHandle::STATUS_RESPONSE)
    {
        http->SetStatus(IHttpHandle::STATUS_OK);
    }

    /// 服务器支持的最高协议版本
    rsp << Protocol;

    /// '状态行'
    rsp << STR_FORMAT(" %s\r\n", GetStatus(http));

    /// '应答头'
    GetHeadItem(http, rsp);

    /// '空行'
    rsp << "\r\n";

    if (pdwHeadSize)
    {
        *pdwHeadSize = rsp.Length();
    }

    /// 如果出错，这里应该先读取错误的内容

    /// 没有回应消息就不读取内容了
    DWORD dwContentLen = http->GetContentLength();
    if (!dwContentLen)
    {
        return;
    }

    /// 如果头部内容大小过大，外面会进行分段读取，这里最多读取实际内容长度
    /// [注] (内容过大时，实际内容体现在应该放的只是资源路径，外面循环Load)
    if (dwContentLen > http->Content().Length())
    {
        dwContentLen = http->Content().Length();
    }

    /// '消息体'
    if ((rsp.Length() + dwContentLen) > DSTREAM_DMEM_MAX_LEN)
    {
        rsp.SetMemMaxLen(rsp.Length() + dwContentLen);
    }

    rsp << CBufferPara(http->Content().Buffer(), dwContentLen);
}

/*******************************************************
  函 数 名: CHttpResponse::GetStatus
  描    述: 获取状态码
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
const char *CHttpResponse::GetStatus(IHttpHandle *http)
{
    if (!http) return Status[IHttpHandle::STATUS_BAD_REQUEST];

    IHttpHandle::STATUS status = http->GetStatus();
    if ((DWORD)status >= (DWORD)ARRAY_SIZE(Status))
    {
        return Status[IHttpHandle::STATUS_INTERNAL_SERVER_ERROR];
    }

    if ( !(*(Status[status])) )
    {
        return Status[IHttpHandle::STATUS_INTERNAL_SERVER_ERROR];;
    }

    return Status[status];
}

/*******************************************************
  函 数 名: CHttpResponse::GetContentType
  描    述: 获取内容类型
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
const char *CHttpResponse::GetContentType(IHttpHandle *http)
{
    if (!http) return MIME[IHttpHandle::MIME_TEXT_HTML];

    if (http->bDynamic())
    {
        IHttpHandle::MIME content_type = http->GetContentType();
        if ((DWORD)content_type >= (DWORD)ARRAY_SIZE(MIME))
        {
            return MIME[IHttpHandle::MIME_TEXT_HTML];
        }

        return MIME[content_type];
    }

    return GetContentType(http->GetURI());
}

/*******************************************************
  函 数 名: CHttpResponse::GetContentType
  描    述: 获取内容类型
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
const char *CHttpResponse::GetContentType(const char *name)
{
    if (!name) return "text/html";

    const char* dot = strrchr( name, '.' );
    if ( dot == (char*) 0 )
        return "text/html";
    if ( strcmp( dot, ".html" ) == 0 || strcmp( dot, ".htm" ) == 0 )
        return "text/html";
    if ( strcmp( dot, ".xhtml" ) == 0 || strcmp( dot, ".xht" ) == 0 )
        return "application/xhtml+xml";
    if ( strcmp( dot, ".jpg" ) == 0 || strcmp( dot, ".jpeg" ) == 0 )
        return "image/jpeg";
    if ( strcmp( dot, ".gif" ) == 0 )
        return "image/gif";
    if ( strcmp( dot, ".png" ) == 0 )
        return "image/png";
    if ( strcmp( dot, ".ico" ) == 0 )
        return "image/x-icon";
    if ( strcmp( dot, ".webp" ) == 0 )
        return "image/webp";
    if ( strcmp( dot, ".css" ) == 0 )
        return "text/css";
    if ( strcmp( dot, ".xml" ) == 0 || strcmp( dot, ".xsl" ) == 0 )
        return "text/xml";
    if ( strcmp( dot, ".au" ) == 0 )
        return "audio/basic";
    if ( strcmp( dot, ".wav" ) == 0 )
        return "audio/wav";
    if ( strcmp( dot, ".avi" ) == 0 )
        return "video/x-msvideo";
    if ( strcmp( dot, ".mov" ) == 0 || strcmp( dot, ".qt" ) == 0 )
        return "video/quicktime";
    if ( strcmp( dot, ".mpeg" ) == 0 || strcmp( dot, ".mpe" ) == 0 )
        return "video/mpeg";
    if ( strcmp( dot, ".vrml" ) == 0 || strcmp( dot, ".wrl" ) == 0 )
        return "model/vrml";
    if ( strcmp( dot, ".midi" ) == 0 || strcmp( dot, ".mid" ) == 0 )
        return "audio/midi";
    if ( strcmp( dot, ".mp3" ) == 0 )
        return "audio/mpeg";
    if ( strcmp( dot, ".ogg" ) == 0 )
        return "application/ogg";
    if ( strcmp( dot, ".pac" ) == 0 )
        return "application/x-ns-proxy-autoconfig";
    return "text/html";
}

/*******************************************************
  函 数 名: CHttpResponse::GetHeadItem
  描    述: 获取消息报头
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpResponse::GetHeadItem(IHttpHandle *http, CDStream &rsp)
{
    if (!http) return;

    time_t now;
    char timebuf[64];

    rsp << STR_FORMAT("Server: %s\r\n", Server);

    now = time( (time_t*) 0 );
    (void) strftime( timebuf, sizeof(timebuf), RFC1123FMT, gmtime( &now ) );
    rsp << STR_FORMAT("Date: %s\r\n", timebuf);

    rsp << STR_FORMAT("Content-Type: %s\r\n", GetContentType(http));

    rsp << STR_FORMAT("Content-Length: %lu\r\n", http->GetContentLength());

    if (http->GetContentEncoding() && ((DWORD)http->GetContentEncoding() < (DWORD)ARRAY_SIZE(Compress)))
        rsp << STR_FORMAT("Content-Encoding: %s\r\n", Compress[http->GetContentEncoding()]);
}

/*******************************************************
  函 数 名: CHttpResponse::IsCompressSupport
  描    述: 是否支持指定的压缩方式
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool CHttpResponse::IsCompressSupport(IHttpHandle *http, IHttpHandle::COMPRESS compress)
{
    if (!http || !compress || ((DWORD)compress >= (DWORD)ARRAY_SIZE(Compress)))
    {
        return false;
    }

    const char *pEncoding = http->GetAcceptEncoding();
    if (!pEncoding)
    {
        return false;
    }

    if (!strstr(pEncoding, Compress[compress]))
    {
        return false;
    }

    return true;
}

/*******************************************************
  函 数 名: CHttpResponse::DeflateContent
  描    述: deflate压缩内容
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpResponse::DeflateContent(IHttpHandle *http)
{
    if (!http || !http->Content().Length())
    {
        return;
    }

    unsigned long zipLen = compressBound(http->Content().Length());
    CDStream sZip((DWORD)zipLen);
    if (!sZip.Buffer())
    {
        return;
    }

    /// zlib格式只有一个2字节的头部，标识这是一个zlib流，并提供解压信息；
    /// 还有一个4字节的尾部，是用来在解压后校验数据完整性的。
    /// IE的deflate编码不识别zlib格式，所以需要去掉zlib头部，只保留中间的压缩数据
    int rc = compress((Bytef *)sZip.Buffer(), &zipLen, (Bytef *)http->Content().Buffer(), http->Content().Length());
    if ((rc != Z_OK) || (zipLen <= 6))
    {
        return;
    }

    http->Content().Clear();
    http->Content() << CBufferPara((BYTE *)sZip.Buffer() + 2, (DWORD)zipLen - 6);
    http->SetContentEncoding(IHttpHandle::COMPRESS_DEFLATE);
    http->SetContentLength(http->Content().Length());
}

/*******************************************************
  函 数 名: CHttpResponse::GzipContent
  描    述: gzip压缩内容
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpResponse::GzipContent(IHttpHandle *http)
{
    if (!http || !http->Content().Length())
    {
        return;
    }

    unsigned long zipLen = (unsigned long)http->Content().Length();
    CDStream sZip((DWORD)zipLen);
    if (!sZip.Buffer())
    {
        return;
    }

    z_stream strm;

    /// 初始化strm结构中的zalloc, zfree, opaque,要求使用默认的内存分配策略
    strm.zalloc = Z_NULL;
    strm.zfree  = Z_NULL;
    strm.opaque = Z_NULL;

    /// 设置输入输出缓冲区
    strm.avail_in = http->Content().Length();
    strm.avail_out = zipLen;
    strm.next_in = (Bytef *)http->Content().Buffer();
    strm.next_out = (Bytef *)sZip.Buffer();

    int err = -1;
    /// 初始化zlib的状态，成功返回Z_OK
    /// deflateInit:zlib格式，deflateInit2:gzip格式
    ///     err = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
    err = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, MAX_WBITS+16, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
    if (err != Z_OK)
    {
        (void)deflateEnd(&strm);
        return;
    }

    /// Z_FINISH表明完成输入，让deflate()完成输出
    err = deflate(&strm, Z_FINISH);
    /// Z_STREAM_END表明所有的输入都已经读完，所有的输出也都产生
    if (err != Z_STREAM_END)
    {
        (void)deflateEnd(&strm);
        return;
    }
    
    /// deflateEnd释放资源，防止内存泄漏
    (void)deflateEnd(&strm);

    /// strm.avail_out表明输出缓冲区剩余的空闲空间大小
    if (zipLen <= strm.avail_out)
    {
        return;
    }

    zipLen -= strm.avail_out;

    http->Content().Clear();
    http->Content() << CBufferPara((BYTE *)sZip.Buffer(), (DWORD)zipLen);
    http->SetContentEncoding(IHttpHandle::COMPRESS_GZIP);
    http->SetContentLength(http->Content().Length());
}

