/// -------------------------------------------------
/// dstring.cpp : 动态字符串实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------

#include "string/dstring.h"
#include "fs/file.h"


/*******************************************************
  函 数 名: CDString::CDString
  描    述: CDString构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDString::CDString()
{
    m_pBuffer = 0;
}

/*******************************************************
  函 数 名: CDString::CDString
  描    述: CDString构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDString::CDString(const char *cpszStr, DWORD dwLen)
{
    m_pBuffer = Copy(cpszStr, dwLen);
}

/*******************************************************
  函 数 名: CDString::CDString
  描    述: CDString构造
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDString::CDString(const CDString &rThis)
{
    m_pBuffer = Copy((const char *)rThis);
}

/*******************************************************
  函 数 名: CDString::~CDString
  描    述: CDString析构
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
CDString::~CDString()
{
    Clear();
}

/*******************************************************
  函 数 名: CDString::Length
  描    述: 获取字符串长度
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDString::Length()
{
    if (!m_pBuffer)
    {
        return 0;
    }

    return (DWORD)strlen(m_pBuffer);
}

/*******************************************************
  函 数 名: CDString::Clear
  描    述: 把字符串清空
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDString::Clear()
{
    if (m_pBuffer)
    {
        DCOP_Free(m_pBuffer);
        m_pBuffer = 0;
    }
}

/*******************************************************
  函 数 名: CDString::Split
  描    述: 字符串分割成字符串数组
  输    入: chrList - 匹配的字符列表
  输    出: strList - 分割的字符串列表
  返    回: 
  修改记录: 
 *******************************************************/
void CDString::Split(const char *chrList, CDArray &strList, bool bNeedSplitChar)
{
    char *pszStr = Copy(m_pBuffer);
    if (!pszStr) return;

    strList.Clear();
    strList.SetNodeSize(sizeof(CDString) + ((bNeedSplitChar)? sizeof(char) : 0));
    strList.SetNodeDelete(StrListFreeHead);

    while (*pszStr)
    {
        if (strList.Append(&pszStr) != SUCCESS)
        {
            break;
        }

        CDString *pNode = (CDString *)strList.Pos(strList.Count() - 1);
        if (!pNode)
        {
            break;
        }

        char *pSplitPos = Located(pszStr, chrList);
        if (!pSplitPos)
        {
            break;
        }

        /// 把分割的字符写入到数组中
        if (bNeedSplitChar) *(char *)(pNode + 1) = *pSplitPos;
        *pSplitPos = '\0';
        pszStr = pSplitPos + 1;
    }

    if (!strList.Count())
    {
        DCOP_Free(pszStr);
    }
}

/*******************************************************
  函 数 名: CDString::Trim
  描    述: 裁剪两侧字符
  输    入: chrList - 匹配的字符列表
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDString::Trim(const char *chrList)
{
    TrimLeft(chrList);
    TrimRight(chrList);
}

/*******************************************************
  函 数 名: CDString::TrimLeft
  描    述: 裁剪左侧字符
  输    入: chrList - 匹配的字符列表
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDString::TrimLeft(const char *chrList, DWORD dwStartPos)
{
    if (!chrList || !m_pBuffer) return;

    DWORD dwListLen = (DWORD)strlen(chrList);
    if (!dwListLen) return;

    DWORD dwStrLen = (DWORD)strlen(m_pBuffer);
    if (!dwStrLen) return;

    /// 遍历左侧进行匹配
    DWORD dwStartIdx = dwStartPos;
    if (dwStartIdx >= dwStrLen)
    {
        return;
    }

    while (dwStartIdx < dwStrLen)
    {
        /// 依次匹配列表中的字符
        bool bMatch = false;
        for (DWORD i = 0; i < dwListLen; ++i)
        {
            if (chrList[i] == m_pBuffer[dwStartIdx])
            {
                bMatch = true;
                break;
            }
        }

        /// 如果列表不匹配当前字符，则退出
        if (!bMatch)
        {
            break;
        }

        /// 如果匹配的话，则继续进行遍历
        ++dwStartIdx;
    }

    /// 如果都不匹配的话，则直接退出
    if (dwStartIdx == dwStartPos)
    {
        return;
    }

    /// 如果全部匹配的话，则清除字符串
    if (dwStartIdx >= dwStrLen)
    {
        m_pBuffer[0] = 0;
        return;
    }

    /// 移动最左侧不匹配的字符串到最开始
    (void)memmove(m_pBuffer + dwStartPos, m_pBuffer + dwStartIdx, dwStrLen - dwStartIdx + 1);
}

/*******************************************************
  函 数 名: CDString::TrimRight
  描    述: 裁剪右侧字符
  输    入: chrList - 匹配的字符列表
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDString::TrimRight(const char *chrList)
{
    if (!chrList || !m_pBuffer) return;

    DWORD dwListLen = (DWORD)strlen(chrList);
    if (!dwListLen) return;

    DWORD dwStrLen = (DWORD)strlen(m_pBuffer);
    if (!dwStrLen) return;

    /// 遍历右侧进行匹配
    DWORD dwStartIdx = dwStrLen;
    while (dwStartIdx)
    {
        /// 依次匹配列表中的字符
        bool bMatch = false;
        for (DWORD i = 0; i < dwListLen; ++i)
        {
            if (chrList[i] == m_pBuffer[dwStartIdx - 1])
            {
                bMatch = true;
                break;
            }
        }

        /// 如果列表不匹配当前字符，则退出
        if (!bMatch)
        {
            break;
        }

        /// 如果匹配的话，则继续进行遍历
        --dwStartIdx;
    }

    /// 如果都不匹配的话，则直接退出
    if (dwStartIdx >= dwStrLen)
    {
        return;
    }

    /// 把最右侧匹配的字符串从尾部清空
    m_pBuffer[dwStartIdx] = '\0';
}

/*******************************************************
  函 数 名: CDString::Insert
  描    述: 插入字符串
  输    入: dwStartPos  - 起始位置
            cpszStr     - 源串
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDString::Insert(DWORD dwStartPos, const char *cpszStr)
{
    if (!cpszStr) return;
    DWORD dwStrOldLen = (m_pBuffer)? (DWORD)strlen(m_pBuffer) : 0;
    DWORD dwStrNewLen = (DWORD)strlen(cpszStr);
    char *pszTmp = (char *)DCOP_Malloc(dwStrOldLen + dwStrNewLen + 1);
    if (!pszTmp)
    {
        return;
    }

    DWORD dwInsertPos = (dwStartPos >= dwStrOldLen)? dwStrOldLen : dwStartPos;
    DWORD dwLeftCount = dwStrOldLen - dwInsertPos;

    if (m_pBuffer)
    {
        if (dwInsertPos)
        {
            (void)memcpy(pszTmp, m_pBuffer, dwInsertPos);
        }
        if (dwLeftCount)
        {
            (void)memcpy(pszTmp + dwInsertPos + dwStrNewLen, m_pBuffer + dwInsertPos, dwLeftCount);
        }
        DCOP_Free(m_pBuffer);
    }

    (void)memcpy(pszTmp + dwInsertPos, cpszStr, dwStrNewLen);
    pszTmp[dwStrOldLen + dwStrNewLen] = '\0';
    m_pBuffer = pszTmp;
}

/*******************************************************
  函 数 名: CDString::Remove
  描    述: 移除字符串
  输    入: dwStartPos  - 起始位置
            dwLen       - 指定长度，为0时或超过源串长度时指后面所有
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDString::Remove(DWORD dwStartPos, DWORD dwLen)
{
    if (!m_pBuffer) return;

    DWORD dwStrLen = (DWORD)strlen(m_pBuffer);
    if (!dwStrLen) return;

    if (dwStartPos >= dwStrLen) return;

    /// 获取剩余的长度
    DWORD dwLeftLen = dwStrLen - dwStartPos;
    if (!dwLen || (dwLen > dwLeftLen))
    {
        dwLen = dwLeftLen;
    }
    dwLeftLen -= dwLen;

    /// 如果有剩余长度的话，则拷贝到前面
    if (dwLeftLen)
    {
        (void)memmove(m_pBuffer + dwStartPos, m_pBuffer + dwStartPos + dwLen, dwLeftLen);
    }

    m_pBuffer[dwStartPos + dwLeftLen] = '\0';
}

/*******************************************************
  函 数 名: CDString::Reverse
  描    述: 反向字符顺序
  输    入: strSub      - 子串
  输    出: 
  返    回: 成功为0开始的正常位置; 错误为TAIL
  修改记录: 
 *******************************************************/
void CDString::Reverse()
{
    if (!m_pBuffer)
    {
        return;
    }

    /// 获取字符串长度
    DWORD dwStrLen = (DWORD)strlen(m_pBuffer);
    if (!dwStrLen)
    {
        return;
    }

    /// 复制新的空间
    char *pszTmp = (char *)DCOP_Malloc(dwStrLen + 1);
    if (!pszTmp)
    {
        return;
    }

    for (DWORD i = 0; i < dwStrLen; ++i)
    {
        pszTmp[i] = m_pBuffer[dwStrLen - i - 1];
    }

    pszTmp[dwStrLen] = '\0';
    DCOP_Free(m_pBuffer);
    m_pBuffer = pszTmp;
}

/*******************************************************
  函 数 名: CDString::Find
  描    述: 查找子串
  输    入: strSub      - 子串
  输    出: 
  返    回: 成功为0开始的正常位置; 错误为TAIL
  修改记录: 
 *******************************************************/
DWORD CDString::Find(const char *strSub)
{
    if (!m_pBuffer)
    {
        return TAIL;
    }

    const char *pcszSubStr = strstr(m_pBuffer, strSub);
    if (!pcszSubStr || (pcszSubStr < m_pBuffer))
    {
        return TAIL;
    }

    return (DWORD)(pcszSubStr - m_pBuffer);
}

/*******************************************************
  函 数 名: CDString::LoadFile
  描    述: 从文件中读取
            (会自动添加结束符'\0')
  输    入: szFile      - 文件名
            dwOffset    - 偏移
            dwCount     - 指定长度，为0时指偏移后的整个文件内容
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
DWORD CDString::LoadFile(const char *szFile, DWORD dwOffset, DWORD dwCount)
{
    if (!szFile) return FAILURE;

    DWORD dwFileLen = DCOP_GetFileLen(szFile);
    if (!dwFileLen)
    {
        return FAILURE;
    }

    if (dwOffset >= dwFileLen)
    {
        return FAILURE;
    }

    DWORD dwReadLen = dwFileLen - dwOffset;
    if (dwCount && (dwReadLen > dwCount))
    {
        dwReadLen = dwCount;
    }

    char *pszBuf = (char *)DCOP_Malloc(dwReadLen + 1);
    if (!pszBuf)
    {
        return FAILURE;
    }

    DWORD dwRc = DCOP_RestoreFromFile(szFile, pszBuf, dwReadLen, dwOffset);
    if (dwRc != SUCCESS)
    {
        DCOP_Free(pszBuf);
        return dwRc;
    }

    pszBuf[dwReadLen] = '\0';
    if (m_pBuffer) DCOP_Free(m_pBuffer);
    m_pBuffer = pszBuf;

    return SUCCESS;
}

/*******************************************************
  函 数 名: CDString::Copy
  描    述: 拷贝字符串
  输    入: cpszStr     - 源串
            dwLen       - 指定长度，为0时使用strlen获取长度
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
char *CDString::Copy(const char *cpszStr, DWORD dwLen)
{
    if (!cpszStr)
    {
        return NULL;
    }

    DWORD dwStrLen = dwLen;
    if (!dwStrLen)
    {
        dwStrLen = (DWORD)strlen(cpszStr);
        if (!dwStrLen) return NULL;
    }

    char *pszTmp = (char *)DCOP_Malloc(dwStrLen + 1);
    if (!pszTmp)
    {
        return NULL;
    }

    (void)memcpy(pszTmp, cpszStr, dwStrLen);
    pszTmp[dwStrLen] = '\0';
    return pszTmp;
}

/*******************************************************
  函 数 名: CDString::Located
  描    述: 从目的字符串中匹配指定的字符列表中任意字符
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
char *CDString::Located(char *pszStr, const char *cpszChr)
{
    if (!pszStr || !cpszChr)
    {
        return NULL;
    }

    /// 循环匹配，如有多个匹配位置，返回最左侧的匹配位置

    char *pszPos = NULL;
    DWORD dwStrLen = (DWORD)strlen(cpszChr);
    for (DWORD i = 0; i < dwStrLen; ++i)
    {
        char *pszPosTmp = strchr(pszStr, cpszChr[i]);
        if (!pszPosTmp)
        {
            continue;
        }

        if (!pszPos || (pszPosTmp < pszPos))
        {
            pszPos = pszPosTmp;
        }
    }

    return pszPos;
}

/*******************************************************
  函 数 名: CDString::StrListFree
  描    述: 字符串数组释放函数
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDString::StrListFree(DWORD index, void *pNode)
{
    if (!pNode) return;

    CDString *pStrObj = (CDString *)pNode;
    pStrObj->Clear();
}

/*******************************************************
  函 数 名: CDString::StrListFreeHead
  描    述: 字符串数组释放函数
            (数组内所有字符串共用一片空间，只释放头节点的字符串空间)
  输    入: 
  输    出: 
  返    回: 
  修改记录: 
 *******************************************************/
void CDString::StrListFreeHead(DWORD index, void *pNode)
{
    if (!index) StrListFree(index, pNode);
}

