/// -------------------------------------------------
/// httpd_handle.cpp : 超文本句柄实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "httpd_handle.h"
#include "BaseMessage.h"


/// -------------------------------------------------
/// 实现基类
/// -------------------------------------------------
DCOP_IMPLEMENT_INSTANCE(CHttpHandle)
    DCOP_IMPLEMENT_INTERFACE(IHttpHandle)
    DCOP_IMPLEMENT_INTERFACE(Instance)
DCOP_IMPLEMENT_INSTANCE_END


/*******************************************************
  函 数 名: IHttpHandle::CreateInstance
  描    述: 创建HTTP句柄
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
IHttpHandle *IHttpHandle::CreateInstance(const char *file, int line)
{
    #undef new
    CHttpHandle *pHttpHandle = new (file, line) CHttpHandle(NULL, 0, 0);
    #define new new(__FILE__, __LINE__)

    return pHttpHandle;
}

/*******************************************************
  函 数 名: CHttpHandle::CHttpHandle
  描    述: CHttpHandle构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CHttpHandle::CHttpHandle(Instance *piParent, int argc, char **argv)
{
    m_method = METHOD_GET;
    m_protocol = PROTO_VER_1_1;

    m_dynamic = false;
    m_status = STATUS_UNSTART;
    m_keep = true;

    m_content_type = MIME_TEXT_HTML;
    m_content_length = 0;
    m_content_encoding = COMPRESS_NO;

    m_condition = DCOP_CONDITION_ANY;

    DCOP_CONSTRUCT_INSTANCE(piParent);
}

/*******************************************************
  函 数 名: CHttpHandle::~CHttpHandle
  描    述: CHttpHandle析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CHttpHandle::~CHttpHandle()
{
    DCOP_DESTRUCT_INSTANCE();
}

/*******************************************************
  函 数 名: CHttpHandle::bCompleted
  描    述: 是否完整
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool CHttpHandle::bCompleted()
{
    if (!m_status)
    {
        return false;
    }

    return (m_content.Length() >= m_content_length);
}

/*******************************************************
  函 数 名: CHttpHandle::ParseParams
  描    述: 解析参数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CHttpHandle::ParseParams()
{
    /// 不同请求方法，参数的位置不一样
    if (m_method == METHOD_GET)
    {
        const char *split = strchr(m_URI, '?');
        if (split)
        {
            m_params.Set(split + 1);
            m_URI.Remove((DWORD)(split - (const char *)m_URI));
        }
    }
    else if (m_method == METHOD_POST)
    {
        m_params.Set((const char *)m_content.Buffer(), m_content.Length());
        m_content.Clear();
    }
    else
    {
    }

    if (!m_params.Length())
    {
        return;
    }

    /// 参数的格式是 "请求参数" @/$ "条件参数" | "系统参数"
    /// "请求参数"和"条件参数"是下发给对象的
    /// "@条件"是相或的任意条件，"$条件"是关键字匹配的唯一条件
    /// "系统参数"是给http本身解析使用的其他参数
    CDArray param_list;
    m_params.Split("@$|", param_list);
    if (!param_list.Count())
    {
        return;
    }

    CDString *pStr = (CDString *)param_list.Pos(HTTP_PARAM_SYS);
    if (!pStr)
    {
        return;
    }

    m_sys_params = *pStr;
    m_sys_params.Split("&", m_sys_param_list);

    pStr = (CDString *)param_list.Pos(HTTP_PARAM_REQ);
    if (!pStr)
    {
        return;
    }

    m_req_params = *pStr;
    m_req_params.Split("&", m_req_param_list);

    pStr = (CDString *)param_list.Pos(HTTP_PARAM_COND);
    if (!pStr)
    {
        return;
    }

    char ch = *(char *)(pStr + 1);
    if (ch == '$') m_condition = DCOP_CONDITION_ONE;
    m_cond_params = *pStr;
    m_cond_params.Split("&", m_cond_param_list);
}

/*******************************************************
  函 数 名: CHttpHandle::GetSysParam
  描    述: 获取系统参数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
const char *CHttpHandle::GetSysParam(const char *param_name)
{
    if (!param_name || !(*param_name))
    {
        return m_params;
    }

    if (!m_sys_param_list.Count())
    {
        return NULL;
    }

    /// 参数一般不多，两位数量级别，使用for循环查找比map等方式还快
    DWORD dwCount = m_sys_param_list.Count();
    for (DWORD i = 0; i < dwCount; ++i)
    {
        CDString *pStr = (CDString *)m_sys_param_list.Pos(i);
        if (!pStr)
        {
            continue;
        }

        const char *cszSplit = strchr(*pStr, '=');
        if (!cszSplit || (cszSplit < (const char *)(*pStr)))
        {
            continue;
        }

        CDString strParamName(*pStr, (DWORD)(cszSplit - (const char *)(*pStr)));
        if (!stricmp(strParamName, param_name))
        {
            return cszSplit + 1;
        }
    }

    return NULL;
}

