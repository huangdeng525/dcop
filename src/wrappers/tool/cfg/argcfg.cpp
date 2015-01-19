/// -------------------------------------------------
/// argcfg.cpp : 参数配置实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "cfg/argcfg.h"
#include "err.h"


/*******************************************************
  函 数 名: CArgCfgTable::CArgCfgTable
  描    述: CArgCfgTable构造函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CArgCfgTable::CArgCfgTable()
{
    m_logPrint = 0;
    m_logPara = 0;
    m_ppCfgItems = 0;
    m_dwCfgCount = 0;
}

/*******************************************************
  函 数 名: CArgCfgTable::CArgCfgTable
  描    述: CArgCfgTable析构函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CArgCfgTable::~CArgCfgTable()
{
    m_logPrint = 0;
    m_logPara = 0;
    if (m_ppCfgItems)
    {
        DCOP_Free(m_ppCfgItems);
        m_ppCfgItems = 0;
    }
    m_dwCfgCount = 0;
}

/*******************************************************
  函 数 名: CArgCfgTable::SetPrintFunc
  描    述: 设置打印函数
  输    入: fnPrint
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CArgCfgTable::SetPrintFunc(LOG_PRINT logPrint, LOG_PARA logPara)
{
    m_logPrint = logPrint;
}

/*******************************************************
  函 数 名: CArgCfgTable::Reg
  描    述: 注册配置项
  输    入: ppCfgItems
            dwCfgCount
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CArgCfgTable::Reg(IArgCfgItemBase **ppCfgItems, DWORD dwCfgCount)
{
    DWORD dwCfgCountTmp = m_dwCfgCount + dwCfgCount;
    IArgCfgItemBase **ppCfgItemsTmp = (IArgCfgItemBase **)DCOP_Malloc(dwCfgCountTmp * sizeof(IArgCfgItemBase *));
    if (!ppCfgItemsTmp)
    {
        return;
    }

    if (m_ppCfgItems)
    {
        (void)memcpy(ppCfgItemsTmp, m_ppCfgItems, m_dwCfgCount * sizeof(IArgCfgItemBase *));
        DCOP_Free(m_ppCfgItems);
        m_ppCfgItems = 0;
    }

    (void)memcpy((char *)ppCfgItemsTmp + m_dwCfgCount * sizeof(IArgCfgItemBase *), 
            ppCfgItems, dwCfgCount * sizeof(IArgCfgItemBase *));
    m_ppCfgItems = ppCfgItemsTmp;
    m_dwCfgCount = dwCfgCountTmp;
}

/*******************************************************
  函 数 名: CArgCfgTable::Cfg
  描    述: 配置入口
  输    入: argc
            argv
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CArgCfgTable::Cfg(int argc, char **argv, bool bIgnoreInvalidArg)
{
    if (!m_ppCfgItems || !m_dwCfgCount)
    {
        return FAILURE;
    }

    for (int iPos = 0; iPos < argc; ++iPos)
    {
        bool bCfg = false;
    
        /// 遍历配置项进行配置
        for (DWORD i = 0; i < m_dwCfgCount; ++i)
        {
            if (!(m_ppCfgItems[i]))
            {
                continue;
            }

            int iRc = m_ppCfgItems[i]->Cfg(iPos, argc, argv);
            if (iRc < 0)
            {
                continue;
            }

            iPos += iRc;
            bCfg = true;
            break;
        }

        /// 如果有无效参数，但是用户设定不能忽略无效参数，那么只能返回错误了
        if (!bIgnoreInvalidArg && !bCfg)
        {
            return FAILURE;
        }
    }

    return SUCCESS;
}

/*******************************************************
  函 数 名: CArgCfgTable::Help
  描    述: 显示帮助信息
  输    入: argc
            argv
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CArgCfgTable::Help(const char *szHelpTitle)
{
    if (!m_logPrint || !m_ppCfgItems || !m_dwCfgCount)
    {
        return;
    }

    m_logPrint(szHelpTitle, m_logPara);

    char szStr[84]; // 一行最多打印的字符(2个空格+16个Name+1个空格+64个Help+1个结束符)

    for (DWORD i = 0; i < m_dwCfgCount; ++i)
    {
        if (!(m_ppCfgItems[i]))
        {
            continue;
        }

        CArgCfgType *pCfgType = m_ppCfgItems[i]->GetCfgType();
        if (!pCfgType)
        {
            continue;
        }

        /// 配置名不能都为空
        if (!(pCfgType->GetSimpleName()) && !(pCfgType->GetFullName()))
        {
            continue;
        }

        /// 配置简名和全名都不为空的话，中间要加一个"逗号"隔开
        DWORD dwSimpleNameLen = (pCfgType->GetSimpleName())? (DWORD)strlen(pCfgType->GetSimpleName()) : 0;
        DWORD dwFullNameLen = (pCfgType->GetFullName())? (DWORD)strlen(pCfgType->GetFullName()) : 0;

        if (!dwSimpleNameLen && !dwFullNameLen)
        {
            continue;
        }

        DWORD dwNameLen = 0;
        char *pszStrName = (char *)DCOP_Malloc(dwSimpleNameLen + 1 + dwFullNameLen + 1);
        if (dwSimpleNameLen)
        {
            dwNameLen += (DWORD)snprintf(pszStrName, dwSimpleNameLen + 1 + dwFullNameLen + 1 - dwNameLen, 
                                        "%s", pCfgType->GetSimpleName());
        }
        if (dwSimpleNameLen && dwFullNameLen)
        {
            dwNameLen += (DWORD)snprintf(pszStrName, dwSimpleNameLen + 1 + dwFullNameLen + 1 - dwNameLen, 
                                        ",");
        }
        if (dwFullNameLen)
        {
            dwNameLen += (DWORD)snprintf(pszStrName, dwSimpleNameLen + 1 + dwFullNameLen + 1 - dwNameLen, 
                                        "%s", pCfgType->GetFullName());
        }

        DWORD dwHelpLen = (pCfgType->GetHelpInfo())? (DWORD)strlen(pCfgType->GetHelpInfo()) : 0;

        /// 格式化输出显示帮助信息
        DWORD dwPosName = 0;
        DWORD dwPosHelp = 0;
        while ((dwPosName < dwNameLen) || (dwPosHelp < dwHelpLen))
        {
            (void)memset(szStr, ' ', sizeof(szStr));
            szStr[sizeof(szStr) - 1] = '\0';

            /// 格式化配置名，每行固定16个
            DWORD dwCpyLen = 0;
            if (dwPosName < dwNameLen)
            {
                dwCpyLen = ((dwNameLen - dwPosName) < 16)? (dwNameLen - dwPosName) : 16;
                (void)memcpy(szStr + 2, pszStrName, dwCpyLen);
                dwPosName += dwCpyLen;
            }

            /// 格式化帮助信息，每行固定64个
            dwCpyLen = 0;
            if (dwPosHelp < dwHelpLen)
            {
                dwCpyLen = ((dwHelpLen - dwPosHelp) < 64)? (dwHelpLen - dwPosHelp) : 64;
                (void)memcpy(szStr + 19, pCfgType->GetHelpInfo(), dwCpyLen);
                dwPosHelp += dwCpyLen;
            }

            szStr[19 + dwCpyLen] = '\0';
            m_logPrint(szStr, m_logPara);
        }

        DCOP_Free(pszStrName);
    }
}

/*******************************************************
  函 数 名: CArgCfgTable::Check
  描    述: 检查必须项是否被配置
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
bool CArgCfgTable::Check()
{
    if (!m_ppCfgItems || !m_dwCfgCount)
    {
        return false;
    }

    for (DWORD i = 0; i < m_dwCfgCount; ++i)
    {
        if (!(m_ppCfgItems[i]))
        {
            continue;
        }

        CArgCfgType *pCfgType = m_ppCfgItems[i]->GetCfgType();
        if (!pCfgType)
        {
            continue;
        }

        if (pCfgType->bMust() && !(m_ppCfgItems[i]->bConfiged()))
        {
            return false;
        }
    }
    
    return true;
}

